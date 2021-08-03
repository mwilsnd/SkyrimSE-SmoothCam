module constructs.decl_offset_group;
import constructs.iconstruct;

import keywords;
import tokenizer : Token, TokenStream;
import tokenizer.tokens;
import tokenizer.utils;
import lex_machine;

import std.conv : to;
import std.uni : toLower;
import std.array : replace;
import std.string;
import std.container.array;

private enum MacroName = "CreateOffsetGroup";
public {
    /** 
     * A declared offset group, holds variable names that were generated
     */
    struct OffsetDecl {
        /// Name of all offset sliders
        string[] offsets;
        /// Name of all interp toggles
        string[] toggles;
    }

    alias VarIDMap = OffsetDecl[string];
}

private struct OffsetGroupDecl {
    Token groupName;
    Token pageName;
    VarIDMap* varMap = null;
    bool noMelee = false;
    bool noRanged = false;
    bool noMagic = false;
    bool noInterpToggles = false;

    /** 
     * Create a SliderSetting #constexpr_struct implementation
     * Params:
     *   settingName = Name of the setting being referenced
     *   groupName = Standing, Walking, Running...
     *   subGroup = Combat:Ranged, Combat:Melee...
     *   niceName = Name of the control shown to the user
     *   desc = Description of the setting
     *   def = Default value
     *   min = Minimum value
     *   max = Maximum value
     * Returns: Generated token stream containing the slider impl
     */
    TokenStream makeSlider(string settingName, string groupName, const(string)* subGroup,
        string niceName, string desc, float def, float min, float max)
    {
        const auto var = subGroup !is null ? replace(*subGroup, ":", "") : "";
        const auto varName = groupName.toLower ~ "_" ~ settingName ~ var;
        const auto settingKey = subGroup !is null ?
            (groupName ~ *subGroup ~ ":" ~ settingName) :
            (groupName ~ ":" ~ settingName);

        TokenStream output;
        // SliderSetting <name> -> [
        output ~= Token(Tok.OtherValue, "SliderSetting");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.OtherValue, varName);
        output ~= Token(Tok.Arrow, "->");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.OpenBrace, "[");
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");

        if (groupName !in (*varMap)) (*varMap)[groupName] = OffsetDecl();
        (*varMap)[groupName].offsets ~= varName;

        // var: value
        output ~= Token(Tok.OtherValue, "settingName");
        output ~= Token(Tok.Colon, ":");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.StringValue, settingKey);
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");

        output ~= Token(Tok.OtherValue, "displayName");
        output ~= Token(Tok.Colon, ":");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.StringValue, niceName);
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");

        output ~= Token(Tok.OtherValue, "desc");
        output ~= Token(Tok.Colon, ":");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.StringValue, desc);
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");

        output ~= Token(Tok.OtherValue, "defaultValue");
        output ~= Token(Tok.Colon, ":");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.NumericValue, to!string(def));
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");
        
        output ~= Token(Tok.OtherValue, "min");
        output ~= Token(Tok.Colon, ":");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.NumericValue, to!string(min));
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");
        
        output ~= Token(Tok.OtherValue, "max");
        output ~= Token(Tok.Colon, ":");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.NumericValue, to!string(max));
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");

        output ~= Token(Tok.OtherValue, "page");
        output ~= Token(Tok.Colon, ":");
        output ~= Token(Tok.Space, " ");
        output ~= pageName;
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");

        // ]
        output ~= Token(Tok.CloseBrace, "]");
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");
        return output;
    }

    /** 
     * Create a ToggleSetting #constexpr_struct implementation
     * Params:
     *   settingName = Name of the setting being referenced
     *   groupName = Standing, Walking, Running...
     *   subGroup = RangedCombat, ...
     *   niceName = Name of the control shown to the user
     *   desc = Description of the setting
     * Returns: Generated token stream containing the toggle impl
     */
    TokenStream makeToggle(string settingName, string groupName, const(string)* subGroup,
        string niceName, string desc)
    {
        const auto var = subGroup !is null ? replace(*subGroup, ":", "") : settingName;
        const auto varName = groupName.toLower ~ "_" ~ var;
        const auto settingKey = settingName ~ groupName ~ (subGroup !is null ? *subGroup : "");

        TokenStream output;

        // ToggleSetting <name> -> [
        output ~= Token(Tok.OtherValue, "ToggleSetting");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.OtherValue, varName);
        output ~= Token(Tok.Arrow, "->");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.OpenBrace, "[");
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");
        (*varMap)[groupName].toggles ~= varName;

        // var: value
        output ~= Token(Tok.OtherValue, "settingName");
        output ~= Token(Tok.Colon, ":");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.StringValue, settingKey);
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");

        output ~= Token(Tok.OtherValue, "displayName");
        output ~= Token(Tok.Colon, ":");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.StringValue, niceName);
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");

        output ~= Token(Tok.OtherValue, "desc");
        output ~= Token(Tok.Colon, ":");
        output ~= Token(Tok.Space, " ");
        output ~= Token(Tok.StringValue, desc);
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");

        output ~= Token(Tok.OtherValue, "page");
        output ~= Token(Tok.Colon, ":");
        output ~= Token(Tok.Space, " ");
        output ~= pageName;
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");

        // ]
        output ~= Token(Tok.CloseBrace, "]");
        output ~= Token(Tok.CharReturn, "\r");
        output ~= Token(Tok.NewLine, "\n");
        return output;
    }

    /** 
     * Create the complete collection of controls for the defined offset group
     * Returns: Token stream containing all control impls
     */
    TokenStream generate() {
        TokenStream output;

        output ~= makeSlider(
            "SideOffset", groupName.value, null,
            "Side Offset", "The amount to move the camera to the right.",
            25, -100, 100
        );
        output ~= makeSlider(
            "UpOffset", groupName.value, null,
            "Up Offset", "The amount to move the camera up.",
            0, -100, 100
        );
        output ~= makeSlider(
            "ZoomOffset", groupName.value, null,
            "Zoom Offset", "The amount to offset the camera zoom by.",
            0, -200, 200
        );
        output ~= makeSlider(
            "FOVOffset", groupName.value, null,
            "FOV Offset", "The amount to offset the camera FOV by." ~
                " Note this will be clamped to a lower bound of 10 and an upper bound of 170.",
            0, -120, 120
        );

        if (!noRanged) {
            const auto ranged = "Combat:Ranged";
            output ~= makeSlider(
                "SideOffset", groupName.value, &ranged,
                "Ranged Side Offset", "The amount to move the camera to the right when in ranged combat.",
                25, -100, 100
            );
            output ~= makeSlider(
                "UpOffset", groupName.value, &ranged,
                "Ranged Up Offset", "The amount to move the camera up when in ranged combat.",
                0, -100, 100
            );
            output ~= makeSlider(
                "ZoomOffset", groupName.value, &ranged,
                "Ranged Zoom Offset", "The amount to offset the camera zoom by when in ranged combat.",
                0, -200, 200
            );
            output ~= makeSlider(
                "FOVOffset", groupName.value, &ranged,
                "Ranged FOV Offset", "The amount to offset the camera FOV by when in ranged combat." ~
                    " Note this will be clamped to a lower bound of 10 and an upper bound of 170.",
                0, -120, 120
            );
        }

        if (!noMagic) {
            const auto magic = "Combat:Magic";
            output ~= makeSlider(
                "SideOffset", groupName.value, &magic,
                "Magic Side Offset", "The amount to move the camera to the right when in magic combat.",
                25, -100, 100
            );
            output ~= makeSlider(
                "UpOffset", groupName.value, &magic,
                "Magic Up Offset", "The amount to move the camera up when in magic combat.",
                0, -100, 100
            );
            output ~= makeSlider(
                "ZoomOffset", groupName.value, &magic,
                "Magic Zoom Offset", "The amount to offset the camera zoom by when in magic combat.",
                0, -200, 200
            );
            output ~= makeSlider(
                "FOVOffset", groupName.value, &magic,
                "Magic FOV Offset", "The amount to offset the camera FOV by when in magic combat." ~
                    " Note this will be clamped to a lower bound of 10 and an upper bound of 170.",
                0, -120, 120
            );
        }

        if (!noMelee) {
            const auto melee = "Combat:Melee";
            output ~= makeSlider(
                "SideOffset", groupName.value, &melee,
                "Melee Side Offset", "The amount to move the camera to the right when in melee combat.",
                25, -100, 100
            );
            output ~= makeSlider(
                "UpOffset", groupName.value, &melee,
                "Melee Up Offset", "The amount to move the camera up when in melee combat.",
                0, -100, 100
            );
            output ~= makeSlider(
                "ZoomOffset", groupName.value, &melee,
                "Melee Zoom Offset", "The amount to offset the camera zoom by when in melee combat.",
                0, -200, 200
            );
            output ~= makeSlider(
                "FOVOffset", groupName.value, &melee,
                "Melee FOV Offset", "The amount to offset the camera FOV by when in melee combat." ~
                    " Note this will be clamped to a lower bound of 10 and an upper bound of 170.",
                0, -120, 120
            );
        }

        if (!noInterpToggles) {
            output ~= makeToggle(
                "Interp", groupName.value, null,
                "Enable Interpolation", "Enables interpolation in this state."
            );
            if (!noRanged) {
                const rc = "RangedCombat";
                output ~= makeToggle(
                    "Interp", groupName.value, &rc,
                    "Enable Ranged Interpolation", "Enables interpolation in this state."
                );
            }
            if (!noMagic) {
                const magc = "MagicCombat";
                output ~= makeToggle(
                    "Interp", groupName.value, &magc,
                    "Enable Magic Interpolation", "Enables interpolation in this state."
                );
            }
            if (!noMelee) {
                const melc = "MeleeCombat";
                output ~= makeToggle(
                    "Interp", groupName.value, &melc,
                    "Enable Melee Interpolation", "Enables interpolation in this state."
                );
            }
        }

        return output;
    }
}

/** 
 * Ex: #CreateOffsetGroup(Sitting, " Sitting", NoMelee, NoRanged, NoMagic)
 * Generates all page elements for configuring the group
 *
 * Args:
 *    required: literal: group name
 *    required: string: page name
 *    opt... : flag literals
 */
final class DeclareOffsetGroup : IConstruct {
    public:
        @disable this(ref return scope DeclareOffsetGroup rhs);
        
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
            struct Memory {
                ulong startIndex = 0;
                OffsetGroupDecl decl;
            }
            enum State {
                ExpectHash,
                ExpectMacro,
                ExpectOpenParen,
                ExpectName,
                ExpectPage,
                ExpectComma,
                ExpectCommaOrCloseParen,
                ExpectOptFlag,
            }
            enum Res {
                Continue,
                BreakInner,
                BreakOuter,
            }

            LexMachine!(
                State, State.ExpectHash,
                Res, Res.Continue,
                Memory
            ) state;

            state.onState(State.ExpectHash, Tok.Hash,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectMacro);
                    state.mem.startIndex = position;
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectMacro, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    if (tok.value == MacroName)
                        state.gotoState(State.ExpectOpenParen);
                    else
                        state.gotoState(State.ExpectHash);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectHash);
                    state.resetMemory();
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectOpenParen, Tok.OpenParen,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectName);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectName, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectComma);
                    state.mem.decl.groupName = tok;
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectComma, Tok.Comma,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectPage);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectPage, Tok.StringValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectCommaOrCloseParen);
                    state.mem.decl.pageName = tok;
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectCommaOrCloseParen, [Tok.Comma, Tok.CloseParen],
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    // Here we can either end the invocation, or pass a list of flag literals
                    if (tok.type == Tok.Comma)
                        state.gotoState(State.ExpectOptFlag);
                    else {
                        auto pre = stream[0..state.mem.startIndex];
                        auto post = stream[position+1..$];

                        state.mem.decl.varMap = &this.varIDs;

                        TokenStream mutated;
                        mutated ~= pre;
                        mutated ~= state.mem.decl.generate()[0..$];
                        mutated ~= post;
                        stream = mutated;
                        return Result!(Res).make(Res.BreakInner);
                    }
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectOptFlag, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    if (tok.value == "NoMelee")
                        state.mem.decl.noMelee = true;
                    else if (tok.value == "NoMagic")
                        state.mem.decl.noMagic = true;
                    else if (tok.value == "NoRanged")
                        state.mem.decl.noRanged = true;
                    else if (tok.value == "NoInterpToggles")
                        state.mem.decl.noInterpToggles = true;
                    else
                        return Result!(Res).fail("Unknown flag '" ~ tok.value ~ "'!");

                    state.gotoState(State.ExpectCommaOrCloseParen);
                    return Result!(Res).make(Res.Continue);
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

        /** 
         * Gets all generated offset group variable names
         * Returns: VarID map
         */
        ref const(VarIDMap) getVarIDs() const return {
            return varIDs;
        }

    private:
        // When generating a new group, var names of all controls are stored here
        VarIDMap varIDs;
}