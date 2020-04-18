/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   Cholesky;
    const int            CholeskySize = 1206;

    extern const char*   CholmodSupport;
    const int            CholmodSupportSize = 1900;

    extern const char*   CMakeLists_txt;
    const int            CMakeLists_txtSize = 694;

    extern const char*   Core;
    const int            CoreSize = 17961;

    extern const char*   Dense;
    const int            DenseSize = 122;

    extern const char*   Eigen;
    const int            EigenSize = 35;

    extern const char*   Eigenvalues;
    const int            EigenvaluesSize = 1822;

    extern const char*   Geometry;
    const int            GeometrySize = 2050;

    extern const char*   Householder;
    const int            HouseholderSize = 874;

    extern const char*   IterativeLinearSolvers;
    const int            IterativeLinearSolversSize = 2083;

    extern const char*   Jacobi;
    const int            JacobiSize = 939;

    extern const char*   LU;
    const int            LUSize = 1433;

    extern const char*   MetisSupport;
    const int            MetisSupportSize = 991;

    extern const char*   OrderingMethods;
    const int            OrderingMethodsSize = 2483;

    extern const char*   PardisoSupport;
    const int            PardisoSupportSize = 1116;

    extern const char*   PaStiXSupport;
    const int            PaStiXSupportSize = 1676;

    extern const char*   QR;
    const int            QRSize = 1317;

    extern const char*   QtAlignedMalloc;
    const int            QtAlignedMallocSize = 945;

    extern const char*   Sparse;
    const int            SparseSize = 919;

    extern const char*   SparseCholesky;
    const int            SparseCholeskySize = 1371;

    extern const char*   SparseCore;
    const int            SparseCoreSize = 2240;

    extern const char*   SparseLU;
    const int            SparseLUSize = 1713;

    extern const char*   SparseQR;
    const int            SparseQRSize = 1222;

    extern const char*   SPQRSupport;
    const int            SPQRSupportSize = 1162;

    extern const char*   StdDeque;
    const int            StdDequeSize = 797;

    extern const char*   StdList;
    const int            StdListSize = 726;

    extern const char*   StdVector;
    const int            StdVectorSize = 803;

    extern const char*   SuperLUSupport;
    const int            SuperLUSupportSize = 2243;

    extern const char*   SVD;
    const int            SVDSize = 1629;

    extern const char*   UmfPackSupport;
    const int            UmfPackSupportSize = 1382;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 30;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
