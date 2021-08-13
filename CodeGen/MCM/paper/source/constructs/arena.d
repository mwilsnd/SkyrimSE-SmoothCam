module constructs.arena;
import constructs.iconstruct;

import keywords;
import tokenizer : Token, TokenStream;
import tokenizer.tokens;
import tokenizer.utils;
import lex_machine;

import std.conv : to;
import std.stdio : writeln;
import std.string;
import std.container.array;

/** 
 * Page of an arena
 *
 * Pages are overlapped arrays on the same underlying arena
 */
struct ArenaPage {
    /// Max slots
    int maxSlots = 0;
    /// Next free index in this arena page
    int nextFreeIndex = 0;
    /// Current alloc count
    int allocCount = 0;
    /// Allocate a new index for something in this page
    int allocateIndex() {
        if (nextFreeIndex + 1 > maxSlots) return -1;
        ++allocCount;
        return nextFreeIndex++;
    }
    /// ctor
    this(int max) {
        maxSlots = max;
    }
}

/** 
 * A generic memory arena
 */
struct ArenaDecl {
    /// Arena data type
    Tok type = Tok.tInt;
    /// Size of arena storage
    uint size = 1;
    /// Var name of the arena
    string name;
    /// Pages in the arena
    ArenaPage[string] pages;
    /// Has been implemented
    bool impl = false;

    invariant {
        assert(
            type == Tok.tInt || type == Tok.tFloat ||
            type == Tok.tBool || type == Tok.tString
        );

        assert(size > 0);
    }

    /// ctor
    this()(Tok ty, uint sz, auto ref const(string) varName) {
        type = ty;
        size = sz;
        name = varName;
    }

    /// Create a new page, or return an existing one if it already exists
    ArenaPage* getPage()(auto ref const(string) pageName) {
        auto it = pageName in pages;
        if (it !is null) return it;

        writeln("Allocating arena page '" ~ pageName ~ "'");
        pages[pageName] = ArenaPage(size);
        return pageName in pages;
    }

    /// Report allocation metrics to console
    void report() const {
        writeln(to!string(pages.length) ~ " pages allocated");
        writeln("Allocation Pressure:");
        foreach (ref const(string) k, ref const(ArenaPage) v; pages) {
            writeln(to!string(k) ~ " - alloc: " ~ to!string(v.allocCount) ~ " maxAlloc: " ~ to!string(v.maxSlots));
        }
    }
}

/** 
 * #DeclArena<int, 255> pagePool
 * ->
 *  int pagePool[];
 *
 * #InitArena(pagePool) ->
 * pagePool = new int[255];
 *
 * In a #constexpr_struct member,
 *  arena<pagePool, page> ref = 0
 * where `page` is a non-real member used for register allocation
 *
 * Thus, all structs with page `General` get assigned ID numbers counting up from 0,
 * acting as indices into the arena array for storing the member `ref`
 * As `page` acts as a guard, ensuring "instances" are never in contention for the same arena index
 *
 * #constexpr_struct `arena<>` members will store an imaginary preallocated arena index at parsing time which
 * is then inlined in every load or store.
 */
final class Arena : IConstruct {
    public:
        @disable this(ref return scope Arena rhs);
        
        /// ctor
        this() {}

        /**
         * Process the token stream and build constructs
         * Params:
         *  stream: Input tokens to parse
         * Returns: Parse result
         */
        Result!bool parse(ref const(TokenStream) stream) @trusted {
            // do nothing
            return Result!(bool).make(true);
        }

        /**
         * Apply generated constructs to the token stream
         * Params:
         *  stream: Mutable token stream to apply to
         * Returns: Apply result
         */
        Result!bool apply(ref TokenStream stream) @trusted {
            auto res = applyDeclArena(stream);
            if (!res.isOk()) return res;

            res = applyInitArena(stream);
            if (!res.isOk()) return res;

            return Result!(bool).make(true);
        }

        ArenaDecl* getNamedArena()(auto ref const(string) name) {
            return name in arenas;
        }

        /// Report allocation metrics to console
        void report() {
            foreach (ref const(ArenaDecl) a; arenas)
                a.report();
        }

    private:
        ArenaDecl[string] arenas;

        Result!bool applyDeclArena(ref TokenStream stream) @trusted {
            struct Memory {
                ulong startIndex = 0;
                Token type;
                uint size = 0;
            }
            enum State {
                ExpectHash,
                ExpectMacroName,
                ExpectOpenAngle,
                ExpectType,
                ExpectComma,
                ExpectSize,
                ExpectCloseAngle,
                ExpectVarName
            }
            enum Res {
                Continue,
                BreakInner,
                BreakOuter
            }

            LexMachine!(
                State, State.ExpectHash,
                Res, Res.Continue,
                Memory
            ) state;

            state.onState(State.ExpectHash, Tok.Hash,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.mem.startIndex = position;
                    state.gotoState(State.ExpectMacroName);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );
            state.onState(State.ExpectMacroName, Tok.OtherValue,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    if (tok.value != "DeclArena") {
                        state.resetMemory();
                        state.gotoState(State.ExpectHash);
                        return Result!(Res).make(Res.Continue);
                    }

                    state.gotoState(State.ExpectOpenAngle);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectHash);
                    return Result!(Res).make(Res.Continue);
                }
            );
            state.onState(State.ExpectOpenAngle, Tok.OpenAngle,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.gotoState(State.ExpectType);
                    return Result!(Res).make(Res.Continue);
                }
            );
            state.onState(State.ExpectType, [Tok.tInt, Tok.tFloat, Tok.tBool, Tok.tString],
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.mem.type = tok;
                    state.gotoState(State.ExpectComma);
                    return Result!(Res).make(Res.Continue);
                }
            );
            state.onState(State.ExpectComma, Tok.Comma,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.gotoState(State.ExpectSize);
                    return Result!(Res).make(Res.Continue);
                }
            );
            state.onState(State.ExpectSize, Tok.NumericValue,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    import std.conv : to;
                    state.mem.size = to!uint(tok.value);
                    state.gotoState(State.ExpectCloseAngle);
                    return Result!(Res).make(Res.Continue);
                }
            );
            state.onState(State.ExpectCloseAngle, Tok.CloseAngle,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.gotoState(State.ExpectVarName);
                    return Result!(Res).make(Res.Continue);
                }
            );
            state.onState(State.ExpectVarName, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    if (tok.value in arenas)
                        return Result!(Res).fail("Duplicate arena decl '" ~ tok.value ~ "'!");

                    auto pre = TokenStream(stream[0..state.mem.startIndex]);
                    auto post = TokenStream(stream[position+1..$]);

                    TokenStream decl;
                    decl ~= state.mem.type;
                    decl ~= Token(Tok.OpenBrace, "[");
                    decl ~= Token(Tok.CloseBrace, "]");
                    decl ~= Token(Tok.Space, " ");
                    decl ~= tok;

                    arenas[tok.value] = ArenaDecl(state.mem.type.type, state.mem.size, tok.value);
                    stream = pre ~ decl ~ post;

                    state.resetMemory();
                    state.gotoState(State.ExpectHash);
                    return Result!(Res).make(Res.BreakInner);
                }
            );

            state.onEOS((ref TokenStream stream) => Result!(Res).make(Res.BreakOuter));

            while (true) {
                const auto res = state.exec(stream);
                if (!res.isOk()) return Result!(bool).failFrom(res);
                if (res.unwrap() == Res.BreakOuter) break;
                state.resetMemory();
                state.gotoState(State.ExpectHash);
            }

            return Result!(bool).make(true);
        }

        Result!bool applyInitArena(ref TokenStream stream) @trusted {
            struct Memory {
                ulong startIndex = 0;
                Token name;
            }
            enum State {
                ExpectHash,
                ExpectMacroName,
                ExpectOpenParen,
                ExpectName,
                ExpectCloseParen,
            }
            enum Res {
                Continue,
                BreakInner,
                BreakOuter
            }

            LexMachine!(
                State, State.ExpectHash,
                Res, Res.Continue,
                Memory
            ) state;

            state.onState(State.ExpectHash, Tok.Hash,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.mem.startIndex = position;
                    state.gotoState(State.ExpectMacroName);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );
            state.onState(State.ExpectMacroName, Tok.OtherValue,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    if (tok.value != "ImplArena") {
                        state.resetMemory();
                        state.gotoState(State.ExpectHash);
                        return Result!(Res).make(Res.Continue);
                    }

                    state.gotoState(State.ExpectOpenParen);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectHash);
                    return Result!(Res).make(Res.Continue);
                }
            );
            state.onState(State.ExpectOpenParen, Tok.OpenParen,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.gotoState(State.ExpectName);
                    return Result!(Res).make(Res.Continue);
                }
            );
            state.onState(State.ExpectName, Tok.OtherValue,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.mem.name = tok;
                    state.gotoState(State.ExpectCloseParen);
                    return Result!(Res).make(Res.Continue);
                }
            );
            state.onState(State.ExpectCloseParen, Tok.CloseParen,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    import std.conv : to;
                    ArenaDecl* it = state.mem.name.value in arenas;
                    if (it is null)
                        return Result!(Res).fail("Undefined arena '" ~ state.mem.name.value ~ "'!");
                    
                    if (it.impl)
                        return Result!(Res).fail("Duplicate initialization of arena '" ~ state.mem.name.value ~ "'!");

                    auto pre = TokenStream(stream[0..state.mem.startIndex]);
                    auto post = TokenStream(stream[position+1..$]);
                    

                    TokenStream output;
                    output ~= state.mem.name;
                    output ~= Token(Tok.Space, " ");
                    output ~= Token(Tok.Assignment, "=");
                    output ~= Token(Tok.Space, " ");
                    output ~= Token(Tok.kNew, "new");
                    output ~= Token(Tok.Space, " ");
                    output ~= Token(it.type, typeToString(it.type));
                    output ~= Token(Tok.OpenBrace, "[");
                    output ~= Token(Tok.NumericValue, to!string(it.size));
                    output ~= Token(Tok.CloseBrace, "]");

                    stream = pre ~ output ~ post;
                    it.impl = true;

                    return Result!(Res).make(Res.BreakInner);
                }
            );

            state.onEOS((ref TokenStream stream) => Result!(Res).make(Res.BreakOuter));

            while (true) {
                const auto res = state.exec(stream);
                if (!res.isOk()) return Result!(bool).failFrom(res);
                if (res.unwrap() == Res.BreakOuter) break;
                state.resetMemory();
                state.gotoState(State.ExpectHash);
            }

            return Result!(bool).make(true);
        }
}