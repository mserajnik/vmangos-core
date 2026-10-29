/*
 * This file is part of the Continued-MaNGOS Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MPQ_H
#define MPQ_H

#include "libmpq/mpq.h"
#include <string.h>
#include <ctype.h>
#include <vector>
#include <iostream>
#include <deque>
#include <cstdint>
#include <cstdio>

using namespace std;

class MPQArchive
{

    public:
        mpq_archive_s* mpq_a;
        string path;

        MPQArchive(const char* filename);
        void close();

        // Returns `false` if the archive has a list file that cannot be read.
        bool GetFileListTo(vector<string>& filelist)
        {
            uint32_t filenum;
            if (libmpq__file_number(mpq_a, "(listfile)", &filenum)) return true;
            libmpq__off_t size, transferred;
            libmpq__file_size_unpacked(mpq_a, filenum, &size);

            // `strtok` needs a terminating NUL, which the list file does not
            // have of its own.
            char* buffer = new char[size + 1];

            int32_t result = libmpq__file_read(mpq_a, filenum, (unsigned char*)buffer, size, &transferred);
            if (result)
            {
                printf("Error reading the list file of archive '%s': %s\n", path.c_str(), libmpq__strerror(result));
                delete[] buffer;
                return false;
            }
            buffer[transferred] = 0;

            char seps[] = "\n";
            char* token;

            token = strtok(buffer, seps);
            while (token != NULL)
            {
                //cout << token << endl;
                // Remove the carriage return of a CRLF line end. The last line
                // can end without one.
                size_t length = strlen(token);
                if (token[length - 1] == '\r')
                    token[length - 1] = 0;
                string s = token;
                filelist.push_back(s);
                token = strtok(NULL, seps);
            }

            delete[] buffer;
            return true;
        }
};
typedef std::deque<MPQArchive*> ArchiveSet;

class MPQFile
{
        //MPQHANDLE handle;
        bool eof;
        char* buffer;
        libmpq__off_t pointer, size;

        // disable copying
        MPQFile(const MPQFile& f) {}
        void operator=(const MPQFile& f) {}

    public:
        MPQFile(const char* filename);    // filenames are not case sensitive
        ~MPQFile() { close(); }
        size_t read(void* dest, size_t bytes);
        size_t getSize() { return size; }
        size_t getPos() { return pointer; }
        char* getBuffer() { return buffer; }
        char* getPointer() { return buffer + pointer; }
        bool isEof() { return eof; }
        void seek(libmpq__off_t offset);
        void seekRelative(libmpq__off_t offset);
        void close();
};

inline void flipcc(char* fcc)
{
    char t;
    t = fcc[0];
    fcc[0] = fcc[3];
    fcc[3] = t;
    t = fcc[1];
    fcc[1] = fcc[2];
    fcc[2] = t;
}

#endif
