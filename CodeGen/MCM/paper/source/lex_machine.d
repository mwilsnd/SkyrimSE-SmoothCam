module lex_machine;
import tokenizer : Token, TokenStream;
import tokenizer.tokens;
import tokenizer.utils;
import result;
import std.conv : to;

/** 
 * A state machine for processing a token stream
 *
 * T is any enum collection.
 * U is any type used as the return result.
 * GoodResult is the U value used to indicate a good result. Any other value breaks out of the state machine.
 * Memory is any struct type used as memory for the state machine.
 * Stream is the type of token stream to operate on
 */
struct LexMachine(
    T, T InitialState,
    U, U GoodResult,
    Memory, Stream = TokenStream,
    bool IgnoreWhitespace = true
)
    if (is(Memory == struct) && is(T == enum))
{
    /** 
     * A callback function run for the given state it is registered with
     * Params:
     *   tok = Current token being evaluated
     *   stream = The stream being operated on
     *   position = The current position in the stream
     * Returns: Result
     */
    alias StateFunctor = Result!U delegate(
        ref const(Token) tok, ref Stream stream, ulong position
    );

    /** 
     * A callback function to run when we reach the end of the stream
     * Params:
     *   stream = The stream being operated on
     * Returns: Result
     */
    alias EOSFunctor = Result!U delegate(
        ref Stream stream
    );

    /** 
     * Execute the state machine on the input stream
     * Params:
     *   stream = Input token stream
     * Returns: Result
     */
    Result!U exec(ref Stream stream) @trusted {
        done = false;
        shouldJump = false;

        for (curPos = 0; curPos < stream.length(); curPos++) {
            if (shouldJump) {
                if (jumpTo != curPos) continue;
                shouldJump = false;
            }

            const Token tok = stream[curPos];
            static if (IgnoreWhitespace)
                if (isWhitespace(tok)) continue;

            auto execFCB(V)(T curState, Token tok, V expected, ref Stream stream, ulong pos) {
                auto fcb = curState in failureFunctors;
                if (fcb !is null) {
                    const auto err = (*fcb)(tok, stream, pos);
                    if (!err.isOk() || err.value != GoodResult)
                        return err;
                
                } else {
                    // No handler, this is an error
                    return Result!(U).fail(
                        "Expected '" ~ to!string(expected) ~ "' but got '" ~ to!string(tok.type) ~ "'!"
                    );
                }

                return Result!(U).make(GoodResult);
            }
            
            // Prefer an exact match
            if (currentState in stateExpect) {
                const auto expected = stateExpect[currentState];
                if (expected != tok.type) {
                    // If we have a failure method, run that
                    const auto err = execFCB(currentState, tok, expected, stream, curPos);
                    if (!err.isOk() || err.value != GoodResult)
                        return err;
                    
                    continue;
                }

            } else if (currentState in stateMultiExpect) {
                // Check in the array
                auto arr = stateMultiExpect[currentState];
                bool matched = false;

                foreach (ref Tok t; arr) {
                    if (t == tok.type) {
                        matched = true;
                        break;
                    }
                }

                if (!matched) {
                    // If we have a failure method, run that
                    const auto err = execFCB(currentState, tok, arr, stream, curPos);
                    if (!err.isOk() || err.value != GoodResult)
                        return err;
                }
                // Otherwise, we matched a token in the list and can run the state functor

            } else {
                // Unhandled state
                assert(0, "Unhandled state!");
            }

            const StateFunctor* cb = currentState in stateFunctors;
            if (cb is null)
                assert(0);
            
            const auto err = (*cb)(tok, stream, curPos);
            if (!err.isOk() || err.value != GoodResult)
                return err;

            if (done) break;
        }

        if (eosFunctor !is null) {
            const auto err = eosFunctor(stream);
            if (!err.isOk() || err.value != GoodResult)
                return err;
        }

        return Result!(U).make(GoodResult);
    }
    
    /** 
     * Register a callback to run for the given state, when the expected token `expect` is seen.
     * Params:
     *   state = State to run on
     *   expect = Expected token to trigger on
     *   functor = Callback to run
     *   matchFailure = Callback to optionally run if the token doesn't match
     */
    void onState(T state, Tok expect, StateFunctor functor, StateFunctor matchFailure = null) @safe nothrow {
        stateExpect[state] = expect;
        stateFunctors[state] = functor;

        if (matchFailure !is null)
            failureFunctors[state] = matchFailure;
    }

    /** 
     * Register a callback to run for the given state, when the expected token `expect` is seen.
     *
     * This variant accepts a list of tokens and will invoke on any match
     * Params:
     *   state = State to run on
     *   expect = Expected token(s) to trigger on
     *   functor = Callback to run
     *   matchFailure = Callback to optionally run if the token doesn't match
     */
    void onState(T state, Tok[] expect, StateFunctor functor, StateFunctor matchFailure = null) @safe nothrow {
        stateMultiExpect[state] = expect;
        stateFunctors[state] = functor;

        if (matchFailure !is null)
            failureFunctors[state] = matchFailure;
    }

    /** 
     * Run the given callback when the end of the stream has been reached
     * Params:
     *   functor = Callback to run
     */
    void onEOS(EOSFunctor functor) @safe nothrow {
        eosFunctor = functor;
    }
    
    /** 
     * Move to a new state
     * Params:
     *   state = New state
     */
    pragma(inline, true)
    void gotoState(T state) @safe @nogc nothrow {
        lastState = currentState;
        currentState = state;
    }

    /** 
     * Call from a state functor to end the state machine execution
     */
    pragma(inline, true)
    void finish() @safe @nogc nothrow {
        done = true;
    }

    /** 
     * Gets the state machine's working memory
     * Returns: Memory
     */
    pragma(inline, true)
    @property
    ref inout(Memory) mem() inout return @trusted @nogc nothrow {
        return memory;
    }

    /** 
     * Reset memory to the initial state
     */
    pragma(inline, true)
    void resetMemory() @trusted @nogc nothrow {
        memory = Memory.init;
    }

    /** 
     * Jump ahead n tokens in the stream
     * Params:
     *   ahead = Number of tokens to jump over
     */
    pragma(inline, true)
    void jump(ulong ahead) @safe @nogc nothrow {
        shouldJump = true;
        jumpTo = ahead + curPos;
    }

    /** 
     * Gets the current state
     * Returns: T
     */
    T getCurrentState() @safe @nogc nothrow const {
        return currentState;
    }

    /** 
     * Look at the next token in the stream ahead of our current position
     * Params:
     *   stream: Stream to peek
     * Returns: Token of valid type or tNone if at end of stream
     */
    Token peekNext(ref Stream stream) @safe @nogc nothrow const {
        if (curPos == stream.length()) return Token(Tok.tNone, "");
        return stream[curPos+1];
    }

    private:
        T currentState = InitialState;
        T lastState = InitialState;

        Tok[T] stateExpect;
        Tok[][T] stateMultiExpect;
        StateFunctor[T] stateFunctors;
        StateFunctor[T] failureFunctors;
        EOSFunctor eosFunctor = null;

        Memory memory;
        bool done = false;
        bool shouldJump = false;
        ulong jumpTo = 0;
        ulong curPos = 0;
}