module bindbc.assimp;

public import bindbc.assimp.types;

version(BindBC_Static) version = BindAssimp_Static;
version(BindAssimp_Static) public import bindbc.assimp.bindstatic;
else public import bindbc.assimp.binddynamic;
