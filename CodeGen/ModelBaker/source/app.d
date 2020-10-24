import std.stdio;
import mesh.basic_writer;

void main(string[] args) {
	if (args.length < 2) {
		writeln("You must provide an input model path");
		return;
	}

	if (args.length < 3) {
		writeln("You must provide an output path");
		return;
	}

	if (!convertMesh(args[1], args[2]))
		writeln("Failed to convert mesh");
}
