module mangler;

/** 
 * Mangles the input string
 *
 * This should generate the exact same string as defined in the C++ module!
 * This is used to cache-bust the game's string cache.
 * Params:
 *   value = String to mangle
 * Returns: Mangled output
 */
string mangle()(auto ref const(string) value) @safe nothrow {
    return "_^@" ~ value ~ "_SmoothCamSetting";
}