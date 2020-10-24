module mesh.basic_writer;
import std.format;
import std.array;
import std.stdio;
import std.string;
import std.math;
import std.algorithm;
import std.conv;
import fs = std.file;

import gfm.math;
import mesh.loader;

/** 
 * Convert the given mesh
 * Params:
 *   inPath = Path to the input mesh
 *   outPath = Path to save the output
 * Returns: Success
 */
bool convertMesh(ref const(string) inPath, ref const(string) outPath) {
    ModelData data;
    if (!ModelLoader.load(inPath, data))
        return false;
    writeln("Data loaded, encoding file...");

    if (fs.exists(outPath))
        fs.remove(outPath);

    writeFile(outPath, data);
    writeln("Done.");
    return true;
}

private:
enum tag = 0xF00590DA;
enum formatVersion = 1;
union f2i {
    float f;
    uint i;
    this(float f) {
        this.f = f;
    }
}

union u32b {
    uint u;
    ubyte[4] b;
}

enum lineWidth = 16;
static uint ptr = 0;

void writeByte(ref const(string) loc, ubyte b) {
    if (ptr != 0)
        fs.append(loc, ",");

    fs.append(loc, format!"0x%x"(b));
    if (++ptr % lineWidth == 0) {
        fs.append(loc, ",\n");
        ptr = 0;
    }
}

void writeU32(ref const(string) loc, uint v) {
    const auto conv = u32b(v);
    writeByte(loc, conv.b[0]);
    writeByte(loc, conv.b[1]);
    writeByte(loc, conv.b[2]);
    writeByte(loc, conv.b[3]);
}

void writeVec4(ref const(string) loc, vec4f vec) {
    f2i x = f2i(vec.x);
    f2i y = f2i(vec.y);
    f2i z = f2i(vec.z);
    f2i w = f2i(vec.w);
    writeU32(loc, x.i);
    writeU32(loc, y.i);
    writeU32(loc, z.i);
    writeU32(loc, w.i);
}

void writeBytes(uint MaxSize)(ref const(string) loc, ref char[MaxSize] bytes) {
    foreach (i, b; bytes) {
        if (i >= MaxSize) break;
        writeByte(loc, cast(ubyte)b);
    }
}

void writeFile(ref const(string) loc, ref ModelData data) {
    writeU32(loc, tag);
    writeU32(loc, formatVersion);
    writeU32(loc, data.numMeshes);
    for (auto i = 0; i < data.numMeshes; i++) {
        writeMesh(loc, data.meshes[i]);
    }
}

void writeMesh(ref const(string) loc, ref MeshData mesh) {
    import core.stdc.string : memcpy;

    // We specify a 128 byte block reserved for the name of a mesh
    // Unused space is simply zero filled
    char[128] name;
    for (auto i = 0; i < 128; i++)
        name[i] = 0;
    memcpy(name.ptr, mesh.name.ptr, mesh.name.length);
    writeBytes!128(loc, name);

    // To make life even easier for now, we unroll the index buffer
    writeU32(loc, mesh.numIndices);
    for (auto i = 0; i < mesh.numIndices; i++) {
        auto vtx = mesh.vertices[mesh.indices[i]];
        // p.w = u, n.x = v
        writeVec4(loc, vtx.p);
        writeVec4(loc, vtx.n);
        writeVec4(loc, vtx.color);
    }
}