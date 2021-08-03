module keywords;
import tokenizer.tokens : Tok;
import result;
import std.string;

alias KeywordMap = Tok[string];
alias TypeMap = Tok[string];

private __gshared KeywordMap keywords;
private __gshared bool kinit = false;

private __gshared TypeMap types;
private __gshared bool tinit = false;

private void loadKeywords() @trusted nothrow {
    keywords["scriptname"] = Tok.kScriptName;
    keywords["extends"] = Tok.kExtends;
    keywords["import"] = Tok.kImport;
    keywords["function"] = Tok.kFunction;
    keywords["endfunction"] = Tok.kEndFunction;
    keywords["event"] = Tok.kEvent;
    keywords["endevent"] = Tok.kEndEvent;
    keywords["if"] = Tok.kIf;
    keywords["elseif"] = Tok.kElseIf;
    keywords["endif"] = Tok.kEndIf;
    keywords["property"] = Tok.kProperty;
    keywords["endproperty"] = Tok.kEndProperty;
    keywords["state"] = Tok.kState;
    keywords["endstate"] = Tok.kEndState;
    keywords["as"] = Tok.kAs;
    keywords["global"] = Tok.kGlobal;
    keywords["native"] = Tok.kNative;
    keywords["auto"] = Tok.kAuto;
    keywords["conditional"] = Tok.kConditional;
    keywords["hidden"] = Tok.kHidden;
    keywords["autoreadonly"] = Tok.kAutoReadOnly;
    keywords["self"] = Tok.kSelf;
    keywords["while"] = Tok.kWhile;
    keywords["endwhile"] = Tok.kEndWhile;
    keywords["length"] = Tok.kLength;
    keywords["new"] = Tok.kNew;
    keywords["parent"] = Tok.kParent;
    keywords["return"] = Tok.kReturn;
    kinit = true;
}

private void loadTypes() @trusted nothrow {
    types["bool"] = Tok.tBool;
    types["float"] = Tok.tFloat;
    types["int"] = Tok.tInt;
    types["string"] = Tok.tString;
    types["literal"] = Tok.tLiteral;
    tinit = true;
}

/** 
 * Gets the map of all language keywords
 * Returns: KeywordMap
 */
ref const(KeywordMap) getKeywords() @trusted nothrow {
    if (!kinit) loadKeywords();
    return keywords;
}

/** 
 * Checks if the given string matches a language keyword
 * Params:
 *   str = Input string
 * Returns: Is keyword
 */
Result!Tok isKeyword(ref const(string) str) @trusted {
    if (!kinit) loadKeywords();
    const auto lower = str.toLower;
    auto it = lower in keywords;
    if (it is null)
        return Result!(Tok).fail(str ~ " is not a keyword!");
    return Result!(Tok).make(*it);
}

/** 
 * Gets the map of all language types
 * Returns: TypeMap
 */
ref const(TypeMap) getTypes() @trusted nothrow {
    if (!tinit) loadTypes();
    return types;
}

/** 
 * Checks if the given string matches a language type
 * Params:
 *   str = Input string
 * Returns: Is type
 */
Result!Tok isType(ref const(string) str) @trusted {
    if (!tinit) loadTypes();
    const auto lower = str.toLower;
    auto it = lower in types;
    if (it is null)
        return Result!(Tok).fail(str ~ " is not a type!");
    return Result!(Tok).make(*it);
}