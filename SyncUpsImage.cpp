// SyncUpsImage.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <algorithm>
#include <filesystem>
#include "FreeImage.h"
#include <exception>

/** Generic image loader
    @param lpszPathName Pointer to the full file name
    @param flag Optional load flag constant
    @return Returns the loaded dib if successful, returns NULL otherwise
*/
FIBITMAP* GenericLoader(const char* lpszPathName, int flag) {
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

    // check the file signature and deduce its format
    // (the second argument is currently not used by FreeImage)
    fif = FreeImage_GetFileType(lpszPathName, 0);
    if (fif == FIF_UNKNOWN) {
        // no signature ?
        // try to guess the file format from the file extension
        fif = FreeImage_GetFIFFromFilename(lpszPathName);
    }
    // check that the plugin has reading capabilities ...
    if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        // ok, let's load the file
        FIBITMAP* dib = FreeImage_Load(fif, lpszPathName, flag);
        // unless a bad file format, we are done !
        return dib;
    }
    return NULL;
}


// return 0 if successful, 1 if failed
int fixTransparent(std::string srcFileName, std::string dstFileName, int scale, unsigned long* transparent, int tolerance)
{
    bool haveTransparent = false;
    int transparentColor8bits = 0;
    RGBQUAD backgroundColor;
    backgroundColor.rgbRed = 255, backgroundColor.rgbGreen = 255, backgroundColor.rgbBlue = 255, backgroundColor.rgbReserved = 255;

    FIBITMAP* srcImg = GenericLoader(srcFileName.data(), 0);
    int srcBytespp = FreeImage_GetLine(srcImg) / FreeImage_GetWidth(srcImg);
    if ( srcBytespp != 1 && srcBytespp != 4 )
    {
        FreeImage_Unload(srcImg);
        return 0;
    }
    BYTE* srcBits = (BYTE*)FreeImage_GetBits(srcImg);
    unsigned srcWidth = FreeImage_GetWidth(srcImg), srcHeight = FreeImage_GetHeight(srcImg);

    if (FreeImage_IsTransparent(srcImg) )
    {
        transparentColor8bits = FreeImage_GetTransparentIndex(srcImg);
    }
    else
    {
        FreeImage_Unload(srcImg);
        return 0;
    }

    if (FreeImage_HasBackgroundColor(srcImg))
    {
        FreeImage_GetBackgroundColor(srcImg, &backgroundColor);
    }

    FIBITMAP* dst = GenericLoader(dstFileName.data(), 0);
    FIBITMAP* dstImg = FreeImage_ConvertTo32Bits(dst);
    FreeImage_Unload(dst);
    int dstBytespp = FreeImage_GetLine(dstImg) / FreeImage_GetWidth(dstImg);
    BYTE* dstBits = (BYTE*)FreeImage_GetBits(dstImg);
    unsigned dstWidth = FreeImage_GetWidth(dstImg), dstHeight = FreeImage_GetHeight(dstImg);

    for (int r = 0; r < srcHeight; ++r)
    {
        srcBits = FreeImage_GetScanLine(srcImg, r);

        for (int c = 0; c < srcWidth; ++c)
        {
            // transparent
            if ((srcBytespp == 1 && transparentColor8bits >= 0 && srcBits[c] == transparentColor8bits)
                || (srcBytespp == 4 && srcBits[c * 4 + FI_RGBA_ALPHA] == 0))
            {
                haveTransparent = true;

                // not only check the transparent pixels but also half the scale pixels around the transparent
                for (int dst_r = 0 - scale / 2; dst_r < scale * 1.5; ++dst_r)
                {
                    if (r * scale + dst_r >= 0 && r * scale + dst_r < dstHeight)
                    {
                        dstBits = FreeImage_GetScanLine(dstImg, r * scale + dst_r);
                        for (int dst_c = 0 - scale / 2; dst_c < scale * 1.5; ++dst_c)
                        {
                            if (c * scale + dst_c >= 0 && c * scale + dst_c < dstWidth)
                            {
                                if (dstBits[c * dstBytespp * scale + dst_c * dstBytespp + FI_RGBA_RED] >= backgroundColor.rgbRed - tolerance
                                    && dstBits[c * dstBytespp * scale + dst_c * dstBytespp + FI_RGBA_RED] <= backgroundColor.rgbRed + tolerance
                                    && dstBits[c * dstBytespp * scale + dst_c * dstBytespp + FI_RGBA_GREEN] >= backgroundColor.rgbGreen - tolerance
                                    && dstBits[c * dstBytespp * scale + dst_c * dstBytespp + FI_RGBA_GREEN] <= backgroundColor.rgbGreen + tolerance
                                    && dstBits[c * dstBytespp * scale + dst_c * dstBytespp + FI_RGBA_BLUE] >= backgroundColor.rgbBlue - tolerance
                                    && dstBits[c * dstBytespp * scale + dst_c * dstBytespp + FI_RGBA_BLUE] <= backgroundColor.rgbBlue + tolerance
                                    && dstBits[c * dstBytespp * scale + dst_c * dstBytespp + FI_RGBA_ALPHA] == 0xff)
                                {
                                    dstBits[c * dstBytespp * scale + dst_c * dstBytespp + FI_RGBA_RED] = 0;
                                    dstBits[c * dstBytespp * scale + dst_c * dstBytespp + FI_RGBA_GREEN] = 0;
                                    dstBits[c * dstBytespp * scale + dst_c * dstBytespp + FI_RGBA_BLUE] = 0;
                                    dstBits[c * dstBytespp * scale + dst_c * dstBytespp + FI_RGBA_ALPHA] = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
   
    if (haveTransparent)
    {
        (*transparent)++;
        FreeImage_Save(FIF_PNG, dstImg, dstFileName.data());
    }

    FreeImage_Unload(srcImg);
    FreeImage_Unload(dstImg);

    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 5)
    {
        std::cout << "Wrong number of parameters. See below on syntax." << std::endl;
        std::cout << "   SyncUpsImage \"destination directory\" \"source directory\" scale_size tolerance" << std::endl;
        std::cout << "   Eg  : SyncUpsImage \"D:\\dest\" \"D:\\src\" 4 2";
        exit(1);
    }

    std::string srcDirName = argv[2];
    std::string dstDirName = argv[1];
    int scale = atoi(argv[3]);
    int tolerance = atoi(argv[4]);

    unsigned long filesSuccess = 0, filesExtChanged = 0, filesFailed = 0, totalFiles = 0, filesTransparent = 0;
    try
    {
        std::filesystem::directory_iterator dst_it(argv[1]), src_it;

        FreeImage_Initialise();

        if (!scale)
        {
            std::cout << "The third argument scale must be a whole number.";
            exit(1);
        }
        if (!tolerance)
        {
            std::cout << "The fourth argument tolerance must be a whole number.";
            exit(1);
        }

        for (const auto& dst : dst_it)
        {
            if (!dst.is_regular_file())
            {
                continue;
            }
            std::string dstPath = std::filesystem::absolute(dst).string();
            int startOfDstFileName = (dstPath.find_last_of("/") == std::string::npos ? 0 : dstPath.find_last_of("/"))
                + (dstPath.find_last_of("\\") == std::string::npos ? 0 : dstPath.find_last_of("\\")) + 1;
            std::string dstFileWoExt = dstPath.substr(startOfDstFileName, dstPath.find_last_of(".") - startOfDstFileName);
            std::string dstFileExt = dstPath.substr(dstPath.find_last_of("."));
            if (dstFileExt.size() <= 0)
            {
                std::cout << "Destination file - " + dstPath + " does not have a file extension. Ignored.";
                filesFailed++;
                totalFiles++;
                continue;
            }

            bool matchingFileFound = false;
            src_it = std::filesystem::directory_iterator(argv[2]);
            for (const auto& src : src_it)
            {
                if (!src.is_regular_file())
                {
                    continue;
                }
                std::string srcPath = std::filesystem::absolute(src).string();
                int startOfSrcFileName = (srcPath.find_last_of("/") == std::string::npos ? 0 : srcPath.find_last_of("/"))
                    + (srcPath.find_last_of("\\") == std::string::npos ? 0 : srcPath.find_last_of("\\")) + 1;
                std::string srcFileWoExt = srcPath.substr(startOfSrcFileName, srcPath.find_last_of(".") - startOfSrcFileName);
                std::string srcFileExt = srcPath.substr(srcPath.find_last_of("."));
                if (srcFileWoExt == dstFileWoExt)
                {
                    // must be before rename else filename will be wrong
                    //int status = fixTransparent(srcPath, dstPath, scale, &filesTransparent, tolerance);
                    int status = 0;

                    matchingFileFound = true;
                    if (srcFileExt != dstFileExt)
                    {
                        std::filesystem::rename(dstPath, dstPath.substr(0, startOfDstFileName) + dstFileWoExt + srcFileExt);
                        filesExtChanged++;
                    }

                    filesFailed += status;
                    filesSuccess += status == 0 ? 1 : 0;
                    break;
                }
            }

            if (!matchingFileFound)
            {
                std::cout << "Unable to find corresponding file in source directory for - " + dstPath << std::endl;
                filesFailed++;
            }
            totalFiles++;
        }
    }
    catch (std::exception&  ex)
    {
        std::cout << "Exception - " << ex.what() << std::endl;
        exit(1);
    }


    std::cout << std::endl << "Scale factor indicated : " << scale << std::endl;
    std::cout << "Total files processed : " << totalFiles << std::endl;
    std::cout << "Total files extension changed : " << filesExtChanged << std::endl;
    std::cout << "Total files successfully processed : " << filesSuccess << std::endl;
    std::cout << "Total files with transparent background in source : " << filesTransparent << std::endl;
    std::cout << "Total files failed : " << filesFailed << std::endl;

    FreeImage_DeInitialise();
    return 0;
}
