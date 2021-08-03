module result;
import std.traits : isCopyable, Unqual;
import std.algorithm : moveEmplace;

/// Return value based error/result
struct Result(T) {
    /// False if there is an error
    bool ok = false;
    /// Error message if ok = false
    string msg = void;
    /// Return value
    Unqual!T value;

    /// explicit ctor
    pragma(inline, true)
    this()(bool ok, auto ref string message, auto ref inout(T) v) @trusted {
        this.ok = ok;
        msg = message;

        static if (isCopyable!T || __traits(isPOD, T))
            value = v;
        else
            moveEmplace(v, value);
    }

    @disable this(this);
    
    /// copy
    static if (isCopyable!T || __traits(isPOD, T))
    pragma(inline, true)
    this(ref return scope inout(Result!T) rhs) inout return @trusted {
        ok = rhs.ok;
        msg = rhs.msg;
        value = rhs.value;
    }

    /// Returns if the result is not an error
    pragma(inline, true)
    bool isOk() const {
        return ok;
    }

    /// Return the value
    static if (isCopyable!T || __traits(isPOD, T))
    pragma(inline, true)
    Unqual!T unwrap() const @trusted {
        assert(ok);
        /// ehhhh
        return cast(Unqual!T)value;
    }

    /// Place the value in `loc`
    pragma(inline, true)
    void unwrapEmplace(ref T loc) @trusted {
        assert(ok);
        moveEmplace(cast(T)value, loc);
    }

    /// Return a failed result with the given message
    static Result!T fail()(auto ref string message) @trusted {
        Result!T res;
        res.ok = false;
        res.msg = message;

        return res;
    }

    /// Return a failed result using the message of other
    static Result!T failFrom(U)(auto ref inout(Result!U) other) @trusted {
        Result!T res;
        res.ok = false;
        res.msg = other.msg;
        return res;
    }

    /// Return an OK result using the given value
    /// Moves the value if not copy-constructable
    static Result!T make()(auto ref inout(T) v) @trusted {
        Result!T res;
        res.ok = true;
        static if (isCopyable!T || __traits(isPOD, T))
            res.value = cast(Unqual!T)v;
        else
            moveEmplace(cast(Unqual!T)v, res.value);
        return res;
    }
}