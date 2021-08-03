module constructs.iconstruct;
public import result;
import tokenizer : Token, TokenStream;

/// A generic language addon construct
interface IConstruct {
    /**
     * Process the token stream and build constructs
     * Params:
     *  tokenStream: Input tokens to parse
     * Returns: Parse result
     */
    Result!bool parse(ref const(TokenStream) tokenStream) @trusted;
    
    /**
     * Apply generated constructs to the token stream
     * Params:
     *  tokenStream: Mutable token stream to apply to
     * Returns: Apply result
     */
    Result!bool apply(ref TokenStream tokenStream) @trusted;
}