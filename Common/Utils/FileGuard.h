/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#include <cstdio>
#include <string>
#include <Exception.h>

namespace Utils
{

class FileGuard final
{
private:
    FILE *file;
public:
    FileGuard(const FileGuard &) = delete;
    FileGuard &operator= (const FileGuard &) = delete;
    FileGuard(FILE *File):file(File){}
    FileGuard(const std::string &Path, const std::string &Mode) throw (Exception)
    {
        if(fopen_s(&file, Path.c_str(), Mode.c_str()))
            throw IOException("Cant open file " + Path);
    }
    FILE *get() const {return file;}
    ~FileGuard() { if(file) fclose(file);}
};

}