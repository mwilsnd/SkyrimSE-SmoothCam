import tokenizer;
import tokenizer.utils;

import constructs.include;
import constructs.const_struct;
import constructs.all_of_struct;
import constructs.struct_invoke_on;
import constructs.struct_invoke_switchifeq;
import constructs.auto_array;
import constructs.decl_offset_group;
import constructs.impl_offset_group;
import constructs.arena;

import fs = std.file;
import std.stdio;
import std.container.array;
import std.path;
import std.utf;

void main(string[] args) {
	writeln("paper - I probably over-engineered this");

	if (args.length < 2) {
		writeln("You must provide an input script path");
		return;
	}

	if (args.length < 3) {
		writeln("You must provide an output script path");
		return;
	}

	string data;
	try {
		data = cast(string)fs.read(args[1]);
	} catch(fs.FileException e) {
		writeln("Failed to read input file: ", e.toString());
		return;
	}

	// Change the working directory to that of the input file
	auto p = absolutePath(args[1]).buildNormalizedPath.dirName;
	fs.chdir(p);

	TokenStream tokens;
	auto err = tokenize(data, tokens);
	if (!err.isOk()) {
		writeln(err.msg);
		return;
	}

	{
		scope auto inc = new Include();
		err = inc.apply(tokens);
		if (!err.isOk()) {
			writeln(err.msg);
			return;
		}
	}

	auto declOfs = new DeclareOffsetGroup();
	err = declOfs.apply(tokens);
	if (!err.isOk()) {
		writeln(err.msg);
		return;
	}

	{
		scope auto implOfs = new ImplOffsetGroup();
		implOfs.setOFSMgr(declOfs);
		err = implOfs.apply(tokens);
		if (!err.isOk()) {
			writeln(err.msg);
			return;
		}
	}

	auto arena = new Arena();
	err = arena.apply(tokens);
	if (!err.isOk()) {
		writeln(err.msg);
		return;
	}

	auto constStruct = new ConstStructParser();
	constStruct.setArena(arena);
	err = constStruct.parse(tokens);
	if (!err.isOk()) {
		writeln(err.msg);
		return;
	}

	arena.report();

	{
		scope auto allOfStruct = new AllOfStruct();
		allOfStruct.setConstStructTool(constStruct);
		err = allOfStruct.apply(tokens);
		if (!err.isOk()) {
			writeln(err.msg);
			return;
		}
	}
	{
		scope auto invokeOn = new StructInvokeOn();
		invokeOn.setConstStructTool(constStruct);
		err = invokeOn.apply(tokens);
		if (!err.isOk()) {
			writeln(err.msg);
			return;
		}
	}
	{
		scope auto autoArray = new AutoArray();
		err = autoArray.apply(tokens);
		if (!err.isOk()) {
			writeln(err.msg);
			return;
		}
	}
	{
		scope auto invokeSwitch = new StructInvokeSwitchIfEq();
		invokeSwitch.setConstStructTool(constStruct);
		err = invokeSwitch.apply(tokens);
		if (!err.isOk()) {
			writeln(err.msg);
			return;
		}
	}

	err = constStruct.apply(tokens);
	if (!err.isOk()) {
		writeln(err.msg);
		return;
	}

	try {
		if (fs.exists(args[2]))
			fs.remove(args[2]);
		
		fs.write(args[2].toUTF8, streamToString(tokens).toUTF8);
	} catch (fs.FileException e) {
		writeln("Failed to write to output file: ", e.toString());
	}
}