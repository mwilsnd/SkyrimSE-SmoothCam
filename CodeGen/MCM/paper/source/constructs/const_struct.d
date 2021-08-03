module constructs.const_struct;
import constructs.iconstruct;

import keywords;
import tokenizer : Token, TokenStream;
import tokenizer.tokens;
import tokenizer.utils;
import lex_machine;
import mangler;

import std.string;
import std.uni : toLower;
import std.container.array;

/// Container for supported base values
union VarValue {
    /// Boolean value
    bool boolValue = void;
    /// Int value
    int intValue = void;
    /// Float value
    float floatValue = void;
    /// String value, also used to hold a literal
    string stringValue = void;
}

/// A member variable in a const struct
struct MemberVar {
    /// The underlying type
    Tok type = Tok.tNone;
    /// If this is a real value or an inlined one
    bool isReal = false;
    /// Should we mangle this string value?
    bool mangle = false;
    /// The value assigned
    VarValue defaultValue;

    invariant {
        assert(
            type == Tok.tBool || type == Tok.tFloat ||
            type == Tok.tInt || type == Tok.tString ||
            type == Tok.tLiteral || type == Tok.tNone
        );
        
        if (type == Tok.tLiteral)
            assert(!isReal);

        if (mangle)
            assert(type == Tok.tString);
    }

    /** 
     * Converts the default value to a string
     * Returns: string
     */
    string valueToString() const {
        import std.conv : to;

        switch (type) {
            case Tok.tBool:
                return to!string(defaultValue.boolValue);
            case Tok.tInt:
                return to!string(defaultValue.intValue);
            case Tok.tFloat:
                return to!string(defaultValue.floatValue);
            case Tok.tLiteral: goto case;
            case Tok.tString:
                return defaultValue.stringValue;
            default: assert(0);
        }
    }
}

/// A member macro in a const struct
struct MemberMacro {
    /// Name of the macro
    string name;
    /// Contents of the macro
    TokenStream code;
}

/// A #constexpr_struct decl
struct ConstStructDecl {
    /// The name of the type
    string typeName;
    /// Default member vars
    MemberVar[string] memberVars;
    /// Macros
    MemberMacro[string] memberMacros;

    /** 
     * Generate processed macro code for the given impl
     * Params:
     *   impl = Implementation of the struct to process and populate
     * Returns: Result
     */
    Result!bool generateMacroCodeForImpl(ref ConstStructImpl impl) const
    in(impl.typeName == this.typeName, "Attempt to generate macro impl for mismatched struct types!")
    {
        struct Memory {
            ulong startIndex = 0;
        }
        enum State {
            ExpectThis,
            ExpectArrow,
            ExpectVar,
        }
        enum Res {
            Continue,
            BreakInner,
            BreakOuter,
        }

        LexMachine!(
            State, State.ExpectThis,
            Res, Res.Continue,
            Memory
        ) state;

        state.onState(State.ExpectThis, Tok.OtherValue,
            (ref const(Token) tok, ref TokenStream stream, ulong position) {
                state.mem.startIndex = position;
                state.gotoState(State.ExpectArrow);
                return Result!(Res).make(Res.Continue);
            },
            (ref const(Token) tok, ref TokenStream stream, ulong position) {
                return Result!(Res).make(Res.Continue);
            }
        );

        state.onState(State.ExpectArrow, Tok.Arrow,
            (ref const(Token) tok, ref TokenStream stream, ulong position) {
                state.gotoState(State.ExpectVar);
                return Result!(Res).make(Res.Continue);
            },
            (ref const(Token) tok, ref TokenStream stream, ulong position) {
                state.resetMemory();
                state.gotoState(State.ExpectThis);
                return Result!(Res).make(Res.Continue);
            }
        );

        state.onState(State.ExpectVar, Tok.OtherValue,
            (ref const(Token) tok, ref TokenStream stream, ulong position) {
                auto pre = stream[0..state.mem.startIndex];
                auto post = stream[position+1..$];

                auto va = impl.memberToActual(tok.value, true);
                if (!va.isOk()) return Result!(Res).failFrom(va);

                TokenStream mutated;
                mutated ~= pre;
                mutated ~= va.unwrap();
                mutated ~= post;
                stream = mutated;
                return Result!(Res).make(Res.BreakInner);
            }
        );

        state.onEOS((ref TokenStream stream) => Result!(Res).make(Res.BreakOuter));

        foreach (string macroName, const MemberMacro mac; memberMacros) {
            // We need to find and replace this-><memberVar> with impl.memberToActual
            // Copy the code
            auto code = TokenStream(mac.code[0..$]);

            // Lex
            while (true) {
                state.resetMemory();
                state.gotoState(State.ExpectThis);

                const auto res = state.exec(code);
                if (!res.isOk()) return Result!(bool).failFrom(res);
                if (res.unwrap() == Res.BreakOuter) break;
                state.resetMemory();
                state.gotoState(State.ExpectThis);
            }

            // Register with impl
            impl.macroCode[macroName] = code;
        }

        return Result!(bool).make(true);
    }
}

/// A #constexpr_struct impl
struct ConstStructImpl {
    /// The name of the struct type
    string typeName;
    /// The variable name of the impl
    string implName;
    /// All vars as defined in the struct, with values optionally set in the impl
    MemberVar[string] memberVars;
    /// Processed macro code, with 'this->' replaced with the unfolded name or inlined value
    TokenStream[string] macroCode;

    /** 
     * Return the processed value of the given member variable
     *
     * Produces either a value literal or unfolded variable name 
     * Params:
     *   memberName = Member to access
     *   isRef = If the type is real, passing true generates a reference to the decl, not the decl itself
     * Returns: actual name/value
     */
    Result!TokenStream memberToActual()(auto ref inout(string) memberName, bool isRef = false) inout return {
        import std.conv : to;

        const MemberVar* it = memberName in memberVars;
        if (it is null)
            return Result!(TokenStream).fail(memberName ~ " is not a member of " ~ implName ~ "!");
        
        TokenStream output;
        if (it.isReal) {
            if (!isRef) {
                output ~= Token(it.type, typeToString(it.type));
                output ~= Token(Tok.Space, " ");
                output ~= Token(Tok.OtherValue, implName ~ "_" ~ memberName);
                output ~= Token(Tok.Space, " ");
                output ~= Token(Tok.Assignment, "=");
                output ~= Token(Tok.Space, " ");
            } else {
                // We just want the name of the variable, this is a reference and not a declaration
                output ~= Token(Tok.OtherValue, implName ~ "_" ~ memberName);
                return Result!(TokenStream).make(output);
            }
        }

        switch (it.type) {
            case Tok.tInt:
                output ~= Token(Tok.NumericValue, to!string(it.defaultValue.intValue));
                break;

            case Tok.tFloat:
                output ~= Token(Tok.NumericValue, to!string(it.defaultValue.floatValue));
                break;

            case Tok.tBool:
                output ~= Token(Tok.NumericValue, to!string(it.defaultValue.boolValue));
                break;
            
            case Tok.tLiteral:
                output ~= Token(Tok.OtherValue, it.defaultValue.stringValue);
                break;

            case Tok.tString:
                if (it.mangle)
                    output ~= Token(Tok.StringValue, mangle(it.defaultValue.stringValue));
                else
                    output ~= Token(Tok.StringValue, it.defaultValue.stringValue);
                    
                break;

            default: assert(0);
        }

        return Result!(TokenStream).make(output);
    }
}

/// Parser generator for #constexpr_struct
final class ConstStructParser : IConstruct {
    public:
        @disable this(ref return scope ConstStructParser rhs);
        
        /// ctor
        this() {}

        /**
         * Process the token stream and build constructs
         * Params:
         *  tokenStream: Input tokens to parse
         * Returns: Parse result
         */
        Result!bool parse(ref const(TokenStream) tokenStream) @trusted {
            auto err = findDecls(tokenStream);
            if (!err.isOk()) return err;

            err = findImpls(tokenStream);
            if (!err.isOk()) return err;

            return Result!(bool).make(true);
        }
        
        /**
         * Apply generated constructs to the token stream
         * Params:
         *  tokenStream: Mutable token stream to apply to
         * Returns: Apply result
         */
        Result!bool apply(ref TokenStream tokenStream) @trusted {
            auto err = stripDecls(tokenStream);
            if (!err.isOk()) return err;

            err = stripImpls(tokenStream);
            if (!err.isOk()) return err;

            err = applyMacroInvocations(tokenStream);
            if (!err.isOk()) return err;

            err = applyDotAccessors(tokenStream);
            if (!err.isOk()) return err;

            return Result!(bool).make(true);
        }

        /** 
         * Gets the struct decl using the given name
         * Params:
         *   name = Type name to lookup
         * Returns: Decl* or null
         */
        ConstStructDecl* getDeclType()(auto ref const(string) name) @safe {
            return name in decls;
        }

        /** 
         * Gets the struct impl using the given name
         * Params:
         *   name = Impl name to lookup
         * Returns: Impl* or null
         */
        ConstStructImpl* getImplType()(auto ref const(string) name) @safe {
            return name in impls;
        }

        /** 
         * Gets a list of all impls of the given decl
         * Params:
         *   decl = The declared struct type to get implementations of
         * Returns: Impl array
         */
        Array!ConstStructImpl getImplsOfDeclType(const ConstStructDecl* decl) @trusted {
            Array!ConstStructImpl list;
            foreach (ref const(string) name, ref ConstStructImpl impl; impls) {
                if (impl.typeName == decl.typeName)
                    list ~= impl;
            }
            return list;
        }

    private:
        ConstStructDecl[string] decls;
        ConstStructImpl[string] impls;

        /** 
         * Find all #constexpr_struct decls in the token stream
         * Params:
         *   tokenStream = Stream to search
         * Returns: Parse result
         */
        Result!bool findDecls(ref const(TokenStream) stream) {
            struct Memory {
                string name;
            }
            enum State {
                ExpectHash,
                ExpectKeyword,
                ExpectName,
                ExpectOpenBrace,
            }
            enum Res {
                Continue,
            }
            LexMachine!(
                State, State.ExpectHash,
                Res, Res.Continue,
                Memory, const(TokenStream)
            ) state;

            state.onState(State.ExpectHash, Tok.Hash,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.gotoState(State.ExpectKeyword);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectKeyword, Tok.OtherValue,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    if (tok.value == "constexpr_struct")
                        state.gotoState(State.ExpectName);
                    else
                        state.gotoState(State.ExpectHash);    
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.gotoState(State.ExpectHash);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectName, Tok.OtherValue,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.gotoState(State.ExpectOpenBrace);
                    state.mem.name = tok.value;
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectOpenBrace, Tok.OpenBrace,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    auto blkStream = TokenStream(stream[position+1..$]);
                    auto block = getBalancedContent(blkStream, Tok.OpenBrace, Tok.CloseBrace);
                    if (!block.isOk()) return Result!(Res).failFrom(block);

                    auto blk = block.unwrap();
                    auto err = digestDeclBlock(state.mem.name, blk);
                    if (!err.isOk()) return Result!(Res).failFrom(err);

                    state.jump(blk.length() + 3);
                    state.resetMemory();
                    state.gotoState(State.ExpectHash);
                    return Result!(Res).make(Res.Continue);
                }
            );

            const auto res = state.exec(stream);
            if (!res.isOk()) return Result!(bool).failFrom(res);

            return Result!(bool).make(true);
        }

        /** 
         * Process the contents of a struct decl block and build metadata for it
         * Params:
         *   name = The name of the struct type
         *   stream = Input stream of all tokens in the block
         * Returns: Parse result
         */
        Result!bool digestDeclBlock()(ref const(string) name, auto ref const(TokenStream) stream) {
            import std.conv : to;

            // Inside a block we can expect the following things:
            //  A member variable decl
            //  A macro decl
            ConstStructDecl decl;
            decl.typeName = name;

            struct Memory {
                bool isMacro = false;
                bool isReal = false;
                bool mangle = false;
                Token type;
                Token name;
            }
            enum State {
                ExpectType,
                ExpectName,
                ExpectAssignment,
                ExpectMacroCode,
                ExpectVarValue,
            }
            enum Res {
                Continue,
                Break,
            }

            LexMachine!(
                State, State.ExpectType,
                Res, Res.Continue,
                Memory, const(TokenStream)
            ) state;

            state.onState(
                State.ExpectType,
                [Tok.OtherValue, Tok.tBool, Tok.tString, Tok.tInt, Tok.tFloat, Tok.tLiteral],
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    if (tok.value == "real") {
                        if (state.mem.isReal)
                            return Result!(Res).fail(
                                "Cannot apply 'real' twice to member '" ~ tok.value ~ "'!"
                            );
                        state.mem.isReal = true;

                    } else if (tok.value == "mangle") {
                        if (state.mem.mangle)
                            return Result!(Res).fail(
                                "Cannot apply 'mangle' twice to member '" ~ tok.value ~ "'!"
                            );
                        state.mem.mangle = true;

                    } else {
                        if (tok.value == "MACRO") {
                            if (state.mem.isReal || state.mem.mangle)
                                return Result!(Res).fail(
                                    "Cannot apply 'real' or 'mangle' to macro '" ~ tok.value ~ "'!"
                                );
                            
                            state.mem.isMacro = true;
                            state.gotoState(State.ExpectName);

                        } else {
                            switch (tok.type) {
                                case Tok.tLiteral:
                                    // Reals are actual variables and require a defined storage class
                                    if (state.mem.isReal)
                                        return Result!(Res).fail("Cannot apply 'real' attr to a literal type!");
                                    goto case;

                                case Tok.tBool: goto case;
                                case Tok.tFloat: goto case;
                                case Tok.tInt: goto case;
                                case Tok.tString:
                                    if (state.mem.mangle && tok.type != Tok.tString)
                                        return Result!(Res).fail("'mangle' can only be applied to strings!");
                                    
                                    state.mem.type = tok;
                                    state.gotoState(State.ExpectName);
                                    break;
                                default:
                                    return Result!(Res).fail("Expected a type but got '" ~ tok.value ~ "'!");
                            }
                        }
                    }

                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectName, Tok.OtherValue,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.mem.name = tok;
                    state.gotoState(State.ExpectAssignment);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectAssignment, Tok.Assignment,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    if (state.mem.isMacro)
                        state.gotoState(State.ExpectMacroCode);
                    else
                        state.gotoState(State.ExpectVarValue);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectVarValue, [Tok.OtherValue, Tok.NumericValue, Tok.StringValue],
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    // Enforce numeric literals for number types
                    if (state.mem.type.type == Tok.tFloat || state.mem.type.type == Tok.tInt) {
                        if (tok.type != Tok.NumericValue)
                            return Result!(Res).fail(
                                "Expected a numeric value, not '" ~ tok.value ~ "'!"
                            );

                    // And strings for strings
                    } else if (state.mem.type.type == Tok.tString) {
                        if (tok.type != Tok.StringValue)
                            return Result!(Res).fail(
                                "Expected a string value, not '" ~ tok.value ~ "'!"
                            );
                    
                    // OtherValue for literals
                    } else if (state.mem.type.type == Tok.tLiteral) {
                        if (tok.type != Tok.OtherValue)
                            return Result!(Res).fail(
                                "Expected a literal value, not '" ~ tok.value ~ "'!"
                            );
                    } else if (state.mem.type.type == Tok.tBool) {
                        if (tok.type != Tok.OtherValue || (tok.value.toLower != "true" && tok.value.toLower != "false"))
                            return Result!(Res).fail(
                                "Expected a boolean value, not '" ~ tok.value ~ "'!"
                            );
                    }

                    MemberVar var;
                    var.type = state.mem.type.type;
                    var.mangle = state.mem.mangle;
                    var.isReal = state.mem.isReal;
                    
                    switch (var.type) {
                        case Tok.tBool:
                            if (tok.value.toLower == "true")
                                var.defaultValue.boolValue = true;
                            else if (tok.value.toLower == "false")
                                var.defaultValue.boolValue = false;
                            else
                                return Result!(Res).fail("Expected a boolean but got '" ~ tok.value ~ "'!");
                            break;

                        case Tok.tInt:
                            var.defaultValue.intValue = to!int(tok.value);
                            break;

                        case Tok.tFloat:
                            var.defaultValue.floatValue = to!float(tok.value);
                            break;

                        case Tok.tLiteral: goto case;
                        case Tok.tString:
                            var.defaultValue.stringValue = tok.value;
                            break;
                        
                        default: assert(0);
                    }

                    decl.memberVars[state.mem.name.value] = var;
                    state.resetMemory();
                    state.gotoState(State.ExpectType);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectMacroCode, Tok.OpenBrace,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    const auto subStream = TokenStream(stream[position+1..$]);
                    auto err = getBalancedContent(subStream, Tok.OpenBrace, Tok.CloseBrace);
                    if (!err.isOk())
                        return Result!(Res).failFrom(err);

                    auto code = err.unwrap();
                    MemberMacro mac;
                    mac.code = trimIndentation(trim!true(code));
                    mac.name = state.mem.name.value;
                    decl.memberMacros[state.mem.name.value] = mac;

                    state.jump(code.length()+2);
                    state.resetMemory();
                    state.gotoState(State.ExpectType);
                    return Result!(Res).make(Res.Continue);
                }
            );

            const auto res = state.exec(stream);
            if (!res.isOk()) return Result!(bool).failFrom(res);

            decls[name] = decl;
            return Result!(bool).make(true);
        }

        /** 
         * Find implementations of #constexpr_struct decls who's type we've seen
         * Params:
         *   tokenStream = Stream to search
         * Returns: Parse result
         */
        Result!bool findImpls(ref const(TokenStream) stream) {
            struct Memory {
                string typeName;
                string implName;
            }
            enum State {
                ExpectType,
                ExpectName,
                ExpectArrow,
                ExpectBody,
            }
            enum Res {
                Continue,
            }
            LexMachine!(
                State, State.ExpectType,
                Res, Res.Continue,
                Memory, const(TokenStream)
            ) state;

            state.onState(State.ExpectType, Tok.OtherValue,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.mem.typeName = tok.value;
                    state.gotoState(State.ExpectName);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectName, Tok.OtherValue,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.mem.implName = tok.value;
                    state.gotoState(State.ExpectArrow);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectType);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectArrow, Tok.Arrow,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.gotoState(State.ExpectBody);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectType);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectBody, Tok.OpenBrace,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    const auto decl = state.mem.typeName in decls;
                    if (decl is null)
                        return Result!(Res).fail(
                            "Cannot create instance of unknown struct type '" ~ state.mem.typeName ~ "'!"
                        );

                    auto subStream = TokenStream(stream[position+1..$]);
                    auto err = getBalancedContent(subStream, Tok.OpenBrace, Tok.CloseBrace);
                    if (!err.isOk())
                        return Result!(Res).failFrom(err);

                    auto blk = err.unwrap();
                    auto implErr = digestImplBlock(decl, state.mem.implName, blk);
                    if (!implErr.isOk())
                        return Result!(Res).failFrom(implErr);
                    
                    state.jump(blk.length() + 3);
                    state.resetMemory();
                    state.gotoState(State.ExpectType);
                    return Result!(Res).make(Res.Continue);
                }
            );

            const auto res = state.exec(stream);
            if (!res.isOk()) Result!(bool).failFrom(res);

            return Result!(bool).make(true);
        }

        /** 
         * Process the contents of a struct impl block and build metadata for it
         * Params:
         *   decl = The decl type
         *   name = The name of the impl
         *   stream = Input stream of all tokens in the block
         * Returns: Parse result
         */
        Result!bool digestImplBlock()(const(ConstStructDecl*) decl, auto ref const(string) name,
            auto ref const(TokenStream) stream)
        {
            import std.conv : to;

            struct Memory {
                string memberName;
            }
            enum State {
                ExpectName,
                ExpectColon,
                ExpectValue,
            }
            enum Res {
                Continue,
            }
            LexMachine!(
                State, State.ExpectName,
                Res, Res.Continue,
                Memory, const(TokenStream)
            ) state;

            ConstStructImpl impl;
            impl.typeName = decl.typeName;
            impl.implName = name;

            // First copy over all defaults from the decl
            foreach (ref const(string) memName, ref const(MemberVar) var; decl.memberVars) {
                impl.memberVars[memName] = var;
            }

            state.onState(State.ExpectName, Tok.OtherValue,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    state.mem.memberName = tok.value;
                    state.gotoState(State.ExpectColon);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectColon, Tok.Colon,
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    // Make sure this is a declared member in the decl
                    if (state.mem.memberName !in impl.memberVars)
                        return Result!(Res).fail(
                            "Attempt to assign to undeclared member '" ~ state.mem.memberName ~ "'!"
                        );

                    state.gotoState(State.ExpectValue);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(
                State.ExpectValue,
                [Tok.BoolValue, Tok.NumericValue, Tok.StringValue, Tok.OtherValue],
                (ref const(Token) tok, ref const(TokenStream) stream, ulong position) {
                    // Here we need to check the declared type of the var
                    // Then expect that exact type - for tLiteral, we want Tok.OtherValue
                    MemberVar* expectType = state.mem.memberName in impl.memberVars;
                    assert(expectType !is null);

                    switch (expectType.type) {
                        case Tok.tInt:
                            if (tok.type != Tok.NumericValue)
                                return Result!(Res).fail("Expected a numeric value but got '" ~ tok.value ~ "'!");

                            expectType.defaultValue.intValue = to!int(tok.value);
                            break;

                        case Tok.tFloat:
                            if (tok.type != Tok.NumericValue)
                                return Result!(Res).fail("Expected a numeric value but got '" ~ tok.value ~ "'!");
                            
                            expectType.defaultValue.floatValue = to!float(tok.value);
                            break;

                        case Tok.tBool:
                            if (tok.type != Tok.BoolValue)
                                return Result!(Res).fail("Expected a boolean value but got '" ~ tok.value ~ "'!");

                            if (tok.value.toLower != "true" && tok.value.toLower != "false")
                                return Result!(Res).fail("Malformed boolean value '" ~ tok.value ~ "'!");

                            expectType.defaultValue.boolValue = tok.value.toLower == "true";
                            break;
                            
                        case Tok.tString:
                            if (tok.type != Tok.StringValue)
                                return Result!(Res).fail("Expected a string value but got '" ~ tok.value ~ "'!");
                            
                            expectType.defaultValue.stringValue = tok.value;
                            break;

                        case Tok.tLiteral:
                            if (tok.type != Tok.OtherValue)
                                return Result!(Res).fail("Expected a literal value but got '" ~ tok.value ~ "'!");
                            
                            expectType.defaultValue.stringValue = tok.value;
                            break;
                        
                        default: assert(0);
                    }

                    state.resetMemory();
                    state.gotoState(State.ExpectName);
                    return Result!(Res).make(Res.Continue);
                }
            );

            // Now for each var we see in the block, assert it is in our list above,
            // then overwrite the value in the impl with what we read
            const auto res = state.exec(stream);
            if (!res.isOk()) Result!(bool).failFrom(res);

            // Register the impl and generate macro code
            auto macGen = decl.generateMacroCodeForImpl(impl);
            if (!macGen.isOk()) return macGen;
            impls[impl.implName] = impl;

            return Result!(bool).make(true);
        }

        /** 
         * Strip #constexpr_struct decls from the token stream
         * Params:
         *   tokenStream = Stream to mutate
         * Returns: Apply result
         */
        Result!bool stripDecls(ref TokenStream stream) {
            struct Memory {
                ulong startIndex = 0;
            }
            enum State {
                ExpectHash,
                ExpectKeyword,
                ExpectName,
                ExpectOpenBrace,
            }
            enum Res {
                Continue,
                BreakInner,
                BreakOuter,
            }
            LexMachine!(
                State, State.ExpectHash,
                Res, Res.Continue,
                Memory, TokenStream
            ) state;

            state.onState(State.ExpectHash, Tok.Hash,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectKeyword);
                    state.mem.startIndex = position;
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectKeyword, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    if (tok.value == "constexpr_struct")
                        state.gotoState(State.ExpectName);
                    else {
                        state.resetMemory();
                        state.gotoState(State.ExpectHash);
                    }
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectHash);
                    state.resetMemory();
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectName, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectOpenBrace);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectOpenBrace, Tok.OpenBrace,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    auto blkStream = TokenStream(stream[position+1..$]);
                    auto block = getBalancedContent(blkStream, Tok.OpenBrace, Tok.CloseBrace);
                    if (!block.isOk()) return Result!(Res).failFrom(block);
                    
                    // Now we know the bounds of this decl, slice it out
                    auto pre = stream[0..state.mem.startIndex];
                    auto post = stream[position+block.unwrap().length()+2..$];
                    
                    // Slice off any trailing empty line left over
                    auto postE = isEmptyLine(TokenStream(post));
                    if (postE.isOk())
                        post = post[postE.unwrap()..$];
                    
                    TokenStream sliced;
                    sliced ~= pre;
                    sliced ~= post;
                    stream = sliced;

                    return Result!(Res).make(Res.BreakInner);
                }
            );

            state.onEOS((ref TokenStream stream) => Result!(Res).make(Res.BreakOuter));
            
            while (true) {
                const auto res = state.exec(stream);
                if (!res.isOk()) Result!(bool).failFrom(res);
                if (res.unwrap() == Res.BreakOuter) break;
                state.resetMemory();
                state.gotoState(State.ExpectHash);
            }

            return Result!(bool).make(true);
        }
        
        /** 
         * Strip #constexpr_struct impls from the token stream
         *
         * If any real members are defined, these are also placed in the now empty location in the stream
         * Params:
         *   tokenStream = Stream to mutate
         * Returns: Apply result
         */
        Result!bool stripImpls(ref TokenStream stream) {
            struct Memory {
                ulong startIndex = 0;
                string implName;
            }
            enum State {
                ExpectType,
                ExpectName,
                ExpectArrow,
                ExpectBody,
            }
            enum Res {
                Continue,
                BreakInner,
                BreakOuter,
            }
            LexMachine!(
                State, State.ExpectType,
                Res, Res.Continue,
                Memory, TokenStream
            ) state;

            state.onState(State.ExpectType, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    if (tok.value in decls) {
                        state.gotoState(State.ExpectName);
                        state.mem.startIndex = position;
                    }
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectName, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.mem.implName = tok.value;
                    state.gotoState(State.ExpectArrow);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectType);
                    state.resetMemory();
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectArrow, Tok.Arrow,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectBody);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectType);
                    state.resetMemory();
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectBody, Tok.OpenBrace,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    auto blkStream = TokenStream(stream[position+1..$]);
                    auto block = getBalancedContent(blkStream, Tok.OpenBrace, Tok.CloseBrace);
                    if (!block.isOk()) return Result!(Res).failFrom(block);

                    // Now we know the bounds of this decl, slice it out
                    auto pre = stream[0..state.mem.startIndex];
                    auto post = stream[position+block.unwrap().length()+2..$];

                    // Slice off any trailing empty line left over
                    auto postE = isEmptyLine(TokenStream(post));
                    if (postE.isOk())
                        post = post[postE.unwrap()..$];

                    TokenStream sliced;
                    sliced ~= pre;

                    // Generate reals for any members that specify so
                    const ConstStructImpl* impl = state.mem.implName in impls;
                    if (impl is null)
                        return Result!(Res).fail(state.mem.implName ~ " was not found!");

                    foreach (ref const(string) name, ref const(MemberVar) var; impl.memberVars) {
                        if (!var.isReal) continue;

                        auto memReal = impl.memberToActual(name);
                        if (!memReal.isOk()) return Result!(Res).failFrom(memReal);
                        sliced ~= memReal.unwrap();
                        sliced ~= Token(Tok.CharReturn, "\r");
                        sliced ~= Token(Tok.NewLine, "\n");
                    }

                    sliced ~= post;
                    stream = sliced;

                    return Result!(Res).make(Res.BreakInner);
                }
            );

            state.onEOS((ref TokenStream stream) => Result!(Res).make(Res.BreakOuter));
            
            while (true) {
                const auto res = state.exec(stream);
                if (!res.isOk()) Result!(bool).failFrom(res);
                if (res.unwrap() == Res.BreakOuter) break;
                state.resetMemory();
                state.gotoState(State.ExpectType);
            }

            return Result!(bool).make(true);
        }

        /** 
         * Matches all macro invocation syntax (->!) and replaces with generated code
         * Params:
         *   stream = Stream to mutate
         * Returns: Apply result
         */
        Result!bool applyMacroInvocations(ref TokenStream stream) {
            struct Memory {
                ulong startIndex = 0;
                string implName;
            }
            enum State {
                ExpectImplName,
                ExpectArrow,
                ExpectExclamation,
                ExpectMacroName,
            }
            enum Res {
                Continue,
                BreakInner,
                BreakOuter,
            }
            LexMachine!(
                State, State.ExpectImplName,
                Res, Res.Continue,
                Memory, TokenStream
            ) state;

            state.onState(State.ExpectImplName, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.mem.startIndex = position;
                    state.mem.implName = tok.value;
                    state.gotoState(State.ExpectArrow);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectArrow, Tok.Arrow,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectExclamation);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectImplName);
                    return Result!(Res).make(Res.Continue);
                }
            );
            
            state.onState(State.ExpectExclamation, Tok.Exclamation,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectMacroName);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectImplName);
                    return Result!(Res).make(Res.Continue);
                }
            );
            
            state.onState(State.ExpectMacroName, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    // Find the impl
                    ConstStructImpl* impl = state.mem.implName in impls;
                    if (impl is null)
                        return Result!(Res).fail("Unknown struct type '" ~ state.mem.implName ~ "'!");
                    
                    // Find the macro
                    auto mac = tok.value in impl.macroCode;
                    if (mac is null)
                        return Result!(Res).fail("Unknown macro '" ~ tok.value ~ "'!");

                    auto pre = stream[0..state.mem.startIndex];
                    auto preE = getIndentation(stream, state.mem.startIndex-1);
                    if (preE.isOk() && preE.unwrap().length > 0) {
                        const auto pl = preE.unwrap().length;
                        const auto idx = state.mem.startIndex - pl;
                        pre = stream[0..idx];
                    }
                    
                    TokenStream mutated;
                    mutated ~= pre;

                    if (preE.isOk() && preE.unwrap().length > 0) {
                        mutated ~= indent(*mac, preE.unwrap()).unwrap();
                    } else {
                        mutated ~= *mac;
                    }

                    mutated ~= stream[position+1..$];
                    stream = mutated;
                    return Result!(Res).make(Res.BreakInner);
                }
            );

            state.onEOS((ref TokenStream stream) => Result!(Res).make(Res.BreakOuter));
            
            while (true) {
                const auto res = state.exec(stream);
                if (!res.isOk()) Result!(bool).failFrom(res);
                if (res.unwrap() == Res.BreakOuter) break;
                state.resetMemory();
                state.gotoState(State.ExpectImplName);
            }
            
            return Result!(bool).make(true);
        }

        /** 
         * Convert impl.member syntax to actual l/rvalue
         * Params:
         *   stream = Stream to mutate
         * Returns: Apply result
         */
        Result!bool applyDotAccessors(ref TokenStream stream) {
            struct Memory {
                ulong startIndex = 0;
                string implName;
            }
            enum State {
                ExpectImpl,
                ExpectDot,
                ExpectMember,
            }
            enum Res {
                Continue,
                BreakInner,
                BreakOuter,
            }
            LexMachine!(
                State, State.ExpectImpl,
                Res, Res.Continue,
                Memory, TokenStream
            ) state;

            state.onState(State.ExpectImpl, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.mem.startIndex = position;
                    state.mem.implName = tok.value;
                    state.gotoState(State.ExpectDot);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectDot, Tok.Dot,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    ConstStructImpl* impl = state.mem.implName in impls;
                    if (impl is null) {
                        state.gotoState(State.ExpectImpl);
                        state.resetMemory();
                    } else
                        state.gotoState(State.ExpectMember);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectImpl);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectMember, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    ConstStructImpl* impl = state.mem.implName in impls;
                    if (impl is null)
                        return Result!(Res).fail("Unknown struct type '" ~ state.mem.implName ~ "'!");
                    
                    auto var = impl.memberToActual(tok.value, true);
                    if (!var.isOk())
                        return Result!(Res).fail("Unknown member '" ~ tok.value ~ "'!");
                    
                    TokenStream mutated;
                    mutated ~= stream[0..state.mem.startIndex];
                    mutated ~= var.unwrap();
                    mutated ~= stream[position+1..$];
                    stream = mutated;

                    return Result!(Res).make(Res.BreakInner);
                }
            );

            state.onEOS((ref TokenStream stream) => Result!(Res).make(Res.BreakOuter));
            
            while (true) {
                const auto res = state.exec(stream);
                if (!res.isOk()) Result!(bool).failFrom(res);
                if (res.unwrap() == Res.BreakOuter) break;
                state.resetMemory();
                state.gotoState(State.ExpectImpl);
            }
            
            return Result!(bool).make(true);
        }
}