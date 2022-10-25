module bindbc.assimp.bindstatic;

version(BindAssimp_Static):

import bindbc.assimp.types;

extern(System) @nogc nothrow {
    size_t aiGetExportFormatCount();
    const(aiExportFormatDesc*) aiGetExportFormatDescription( size_t pIndex);
    void aiReleaseExportFormatDescription( const aiExportFormatDesc *desc );
    void aiCopyScene(const aiScene* pIn, aiScene** pOut);
    void aiFreeScene(const aiScene* pIn);
    aiReturn aiExportScene( const aiScene* pScene,
            const char* pFormatId,
            const char* pFileName,
            uint pPreprocessing);
    aiReturn aiExportSceneEx( const aiScene* pScene,
            const char* pFormatId,
            const char* pFileName,
            aiFileIO* pIO,
            uint pPreprocessing );
    const(aiExportDataBlob*) aiExportSceneToBlob( const aiScene* pScene, const char* pFormatId,
            uint pPreprocessing );
    void aiReleaseExportBlob( const aiExportDataBlob* pData );
    const(aiScene*) aiImportFile(
            const char* pFile,
            uint pFlags);
    const(aiScene*) aiImportFileEx(
            const char* pFile,
            uint pFlags,
            aiFileIO* pFS);
    const(aiScene*) aiImportFileExWithProperties(
            const char* pFile,
            uint pFlags,
            aiFileIO* pFS,
            const aiPropertyStore* pProps);
    const(aiScene*) aiImportFileFromMemory(
            const char* pBuffer,
            uint pLength,
            uint pFlags,
            const char* pHint);
    const(aiScene*) aiImportFileFromMemoryWithProperties(
            const char* pBuffer,
            uint pLength,
            uint pFlags,
            const char* pHint,
            const aiPropertyStore* pProps);
    const(aiScene*) aiApplyPostProcessing(
            const aiScene* pScene,
            uint pFlags);
    aiLogStream aiGetPredefinedLogStream(
            aiDefaultLogStream pStreams,
            const char* file);
    void aiAttachLogStream(
            const aiLogStream* stream);
    void aiEnableVerboseLogging(aiBool d);
    aiReturn aiDetachLogStream(
            const aiLogStream* stream);
    void aiDetachAllLogStreams();
    void aiReleaseImport(
            const aiScene* pScene);
    const(char*) aiGetErrorString();
    aiBool aiIsExtensionSupported(
            const char* szExtension);
    void aiGetExtensionList(
            aiString* szOut);
    void aiGetMemoryRequirements(
            const aiScene* pIn,
            aiMemoryInfo* _in);
    aiPropertyStore* aiCreatePropertyStore();
    void aiReleasePropertyStore(aiPropertyStore* p);
    void aiSetImportPropertyInteger(
            aiPropertyStore* store,
            const char* szName,
            int value);
    void aiSetImportPropertyFloat(
            aiPropertyStore* store,
            const char* szName,
            ai_real value);
    void aiSetImportPropertyString(
            aiPropertyStore* store,
            const char* szName,
            const aiString* st);
    void aiSetImportPropertyMatrix(
            aiPropertyStore* store,
            const char* szName,
            const aiMatrix4x4* mat);
    void aiCreateQuaternionFromMatrix(
            aiQuaternion* quat,
            const aiMatrix3x3* mat);
    void aiDecomposeMatrix(
            const aiMatrix4x4* mat,
            aiVector3D* scaling,
            aiQuaternion* rotation,
            aiVector3D* position);
    void aiTransposeMatrix4(
            aiMatrix4x4* mat);
    void aiTransposeMatrix3(
            aiMatrix3x3* mat);
    void aiTransformVecByMatrix3(
            aiVector3D* vec,
            const aiMatrix3x3* mat);
    void aiTransformVecByMatrix4(
            aiVector3D* vec,
            const aiMatrix4x4* mat);
    void aiMultiplyMatrix4(
            aiMatrix4x4* dst,
            const aiMatrix4x4* src);
    void aiMultiplyMatrix3(
            aiMatrix3x3* dst,
            const aiMatrix3x3* src);
    void aiIdentityMatrix3(
            aiMatrix3x3* mat);
    void aiIdentityMatrix4(
            aiMatrix4x4* mat);
    size_t aiGetImportFormatCount();
    const(aiImporterDesc*) aiGetImportFormatDescription( size_t pIndex);
    const(aiImporterDesc*) aiGetImporterDesc( const char *extension );
    aiReturn aiGetMaterialProperty(
            const aiMaterial* pMat,
            const char* pKey,
            uint type,
            uint index,
            const aiMaterialProperty** pPropOut);
    aiReturn aiGetMaterialFloatArray(
            const aiMaterial* pMat,
            const char* pKey,
            uint type,
            uint index,
            ai_real* pOut,
            uint* pMax);
    aiReturn aiGetMaterialFloat(
            const aiMaterial* pMat,
            const char* pKey,
            uint type,
            uint index,
            ai_real* pOut)
    {
        return aiGetMaterialFloatArray(pMat, pKey, type, index, pOut, null);
    }
    aiReturn aiGetMaterialIntegerArray(const aiMaterial* pMat,
            const char* pKey,
            uint  type,
            uint  index,
            int* pOut,
            uint* pMax);
    aiReturn aiGetMaterialInteger(const aiMaterial* pMat,
            const char* pKey,
            uint  type,
            uint  index,
            int* pOut,
            uint* pMax);
    /*aiReturn aiGetMaterialInteger(const aiMaterial* pMat,
            const char* pKey,
            uint  type,
            uint  index,
            int* pOut)
    {
        return aiGetMaterialIntegerArray(pMat, pKey, type, index, pOut, null);
    }*/
    aiReturn aiGetMaterialColor(const aiMaterial* pMat,
            const char* pKey,
            uint type,
            uint index,
            aiColor4D* pOut);
    aiReturn aiGetMaterialUVTransform(const aiMaterial* pMat,
            const char* pKey,
            uint type,
            uint index,
            aiUVTransform* pOut);
    aiReturn aiGetMaterialString(const aiMaterial* pMat,
            const char* pKey,
            uint type,
            uint index,
            aiString* pOut);
    uint aiGetMaterialTextureCount(const aiMaterial* pMat,
            aiTextureType type);
    aiReturn aiGetMaterialTexture(const aiMaterial* mat,
            aiTextureType type,
            uint  index,
            aiString* path,
            aiTextureMapping* mapping,
            uint* uvindex,
            ai_real* blend,
            aiTextureOp* op,
            aiTextureMapMode* mapmode,
            uint* flags);
    const(char*)  aiGetLegalString();
    uint aiGetVersionMinor();
    uint aiGetVersionMajor();
    uint aiGetVersionRevision();
    const(char*) aiGetBranchName();
    uint aiGetCompileFlags();
}
