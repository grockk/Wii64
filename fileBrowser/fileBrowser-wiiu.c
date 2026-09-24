#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <whb/sdcard.h>

#include "fileBrowser.h"
#include "fileBrowser-libfat.h"

fileBrowser_file topLevel_libfat_Default = {
        "/vol/external01/wii64/roms", 0, 0, 0, FILE_BROWSER_ATTR_DIR
};

fileBrowser_file topLevel_libfat_USB = {
        "/vol/external01/wii64/roms", 0, 0, 0, FILE_BROWSER_ATTR_DIR
};

fileBrowser_file saveDir_libfat_Default = {
        "/vol/external01/wii64/saves", 0, 0, 0, FILE_BROWSER_ATTR_DIR
};

fileBrowser_file saveDir_libfat_USB = {
        "/vol/external01/wii64/saves", 0, 0, 0, FILE_BROWSER_ATTR_DIR
};

static int wiiu_is_rom(const char *name)
{
        const char *extension = strrchr(name, '.');
        if (!extension)
                return 0;

        return strcasecmp(extension, ".v64") == 0 ||
               strcasecmp(extension, ".z64") == 0 ||
               strcasecmp(extension, ".n64") == 0 ||
               strcasecmp(extension, ".bin") == 0;
}

static int wiiu_read_dir(fileBrowser_file *file, fileBrowser_file **entries,
                         int recursive, int n64only)
{
        DIR *directory = opendir(file->name);
        struct dirent *entry;
        int count = 0;

        if (!directory)
                return FILE_BROWSER_ERROR;

        while ((entry = readdir(directory)) != NULL) {
                fileBrowser_file child;
                struct stat info;

                if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                        continue;

                memset(&child, 0, sizeof(child));
                if (snprintf(child.name, sizeof(child.name), "%s/%s", file->name,
                             entry->d_name) >= (int)sizeof(child.name))
                        continue;
                if (stat(child.name, &info) < 0)
                        continue;

                child.size = (unsigned int)info.st_size;
                child.attr = S_ISDIR(info.st_mode) ? FILE_BROWSER_ATTR_DIR : 0;

                if (child.attr == FILE_BROWSER_ATTR_DIR && recursive) {
                        int nested = wiiu_read_dir(&child, entries, recursive,
                                                   n64only);
                        if (nested > 0)
                                count += nested;
                        continue;
                }

                if (child.attr == 0 && n64only && !wiiu_is_rom(child.name))
                        continue;

                *entries = realloc(*entries,
                                   (size_t)(count + 1) * sizeof(**entries));
                if (!*entries) {
                        closedir(directory);
                        return FILE_BROWSER_ERROR;
                }
                (*entries)[count++] = child;
        }

        closedir(directory);
        return count;
}

int fileBrowser_libfat_readDir(fileBrowser_file *file,
                               fileBrowser_file **entries, int recursive,
                               int n64only)
{
        return wiiu_read_dir(file, entries, recursive, n64only);
}

int fileBrowser_libfat_seekFile(fileBrowser_file *file, unsigned int where,
                                unsigned int type)
{
        if (type == FILE_BROWSER_SEEK_SET)
                file->offset = where;
        else if (type == FILE_BROWSER_SEEK_CUR)
                file->offset += where;
        else
                file->offset = file->size + where;
        return 0;
}

int fileBrowser_libfat_readFile(fileBrowser_file *file, void *buffer,
                                unsigned int length)
{
        FILE *stream = fopen(file->name, "rb");
        size_t read;

        if (!stream || fseek(stream, (long)file->offset, SEEK_SET) != 0) {
                if (stream)
                        fclose(stream);
                return FILE_BROWSER_ERROR;
        }

        read = fread(buffer, 1, length, stream);
        fclose(stream);
        file->offset += (unsigned int)read;
        return (read == length) ? 0 : FILE_BROWSER_ERROR;
}

int fileBrowser_libfat_writeFile(fileBrowser_file *file, void *buffer,
                                 unsigned int length)
{
        FILE *stream = fopen(file->name, "r+b");
        size_t written;

        if (!stream)
                stream = fopen(file->name, "wb");
        if (!stream || fseek(stream, (long)file->offset, SEEK_SET) != 0) {
                if (stream)
                        fclose(stream);
                return FILE_BROWSER_ERROR;
        }

        written = fwrite(buffer, 1, length, stream);
        fclose(stream);
        file->offset += (unsigned int)written;
        if (file->offset > file->size)
                file->size = file->offset;
        return (written == length) ? 0 : FILE_BROWSER_ERROR;
}

int fileBrowser_libfat_deleteFile(fileBrowser_file *file)
{
        return remove(file->name) == 0 ? 0 : FILE_BROWSER_ERROR;
}

int fileBrowser_libfat_init(fileBrowser_file *file)
{
        struct stat info;

        if (!WHBMountSdCard())
                return FILE_BROWSER_ERROR;
        if (stat(file->name, &info) < 0 && errno == ENOENT)
                mkdir(file->name, 0777);
        return stat(file->name, &info) == 0 ? 0 : FILE_BROWSER_ERROR;
}

int fileBrowser_libfat_deinit(fileBrowser_file *file)
{
        (void)file;
        return 0;
}

int fileBrowser_libfatROM_readFile(fileBrowser_file *file, void *buffer,
                                   unsigned int length)
{
        return fileBrowser_libfat_readFile(file, buffer, length);
}

int fileBrowser_libfatROM_deinit(fileBrowser_file *file)
{
        return fileBrowser_libfat_deinit(file);
}

void pauseRemovalThread(void) {}
void continueRemovalThread(void) {}