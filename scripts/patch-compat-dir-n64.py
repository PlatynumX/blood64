#!/usr/bin/env python3
from pathlib import Path
import sys

def patch(path):
    s = path.read_text()

    old = "#elif defined(_MSC_VER)\n#else\n# include <dirent.h>\n"
    new = "#elif defined(_MSC_VER)\n#elif defined(N64) || defined(__N64__)\n# include <dir.h>\n#else\n# include <dirent.h>\n"
    if old in s:
        s = s.replace(old, new, 1)
    elif "# include <dir.h>" not in s:
        raise SystemExit("expected dirent include block not found")

    start = s.find("typedef struct {\n#ifdef _MSC_VER\n\tHANDLE hfind;")
    end = s.find("\n\n\nchar *Bstrtoken(", start)
    if start < 0 or end < 0:
        if "dir_findfirst(name, &dirr->ldir)" in s:
            print("already patched")
            return
        raise SystemExit("exact BDIR block not found")

    block = r'''typedef struct {
#ifdef _MSC_VER
	HANDLE hfind;
	WIN32_FIND_DATA fid;
#elif defined(N64) || defined(__N64__)
	dir_t ldir;
	char *root;
#else
	DIR *dir;
	int rootlen;
	char *work;
	int worklen;
#endif
	struct Bdirent info;
	int status;
} BDIR_real;

BDIR* Bopendir(const char *name)
{
	BDIR_real *dirr;
#ifdef _MSC_VER
	char *tname, *tcurs;
#endif

	dirr = (BDIR_real*)malloc(sizeof(BDIR_real));
	if (!dirr) return NULL;
	memset(dirr, 0, sizeof(BDIR_real));

#ifdef _MSC_VER
	tname = (char*)malloc(strlen(name) + 4 + 1);
	if (!tname) { free(dirr); return NULL; }
	strcpy(tname, name);
	for (tcurs = tname; *tcurs; tcurs++) ;
	tcurs--;
	while (*tcurs == ' ' && tcurs>tname) tcurs--;
	if (*tcurs != '/' && *tcurs != '\\') *(++tcurs) = '/';
	*(++tcurs) = '*'; *(++tcurs) = '.'; *(++tcurs) = '*'; *(++tcurs) = 0;
	dirr->hfind = FindFirstFile(tname, &dirr->fid);
	free(tname);
	if (dirr->hfind == INVALID_HANDLE_VALUE) { free(dirr); return NULL; }
	dirr->status = 0;
#elif defined(N64) || defined(__N64__)
	dirr->root = strdup(name);
	if (!dirr->root) { free(dirr); return NULL; }
	{
		int ret = dir_findfirst(name, &dirr->ldir);
		if (ret == -1) {
			/* Existing empty directory: valid handle, no first entry. */
			dirr->status = -1;
			return (BDIR*)dirr;
		}
		if (ret < 0) {
			free(dirr->root);
			free(dirr);
			return NULL;
		}
	}
	/* First entry is already buffered by dir_findfirst(). */
	dirr->status = 0;
#else
	dirr->dir = opendir(name);
	if (dirr->dir == NULL) { free(dirr); return NULL; }
	dirr->rootlen = strlen(name);
	dirr->worklen = dirr->rootlen + 1 + 64 + 1;
	dirr->work = (char *)malloc(dirr->worklen);
	if (!dirr->work) {
		closedir(dirr->dir);
		free(dirr);
		return NULL;
	}
	strcpy(dirr->work, name);
	strcat(dirr->work, "/");
	dirr->status = 0;
#endif
	return (BDIR*)dirr;
}

struct Bdirent* Breaddir(BDIR *dir)
{
	BDIR_real *dirr = (BDIR_real*)dir;
#ifdef _MSC_VER
	LARGE_INTEGER tmp;
	if (dirr->status > 0) {
		if (FindNextFile(dirr->hfind, &dirr->fid) == 0) {
			dirr->status = -1;
			return NULL;
		}
	}
	dirr->info.namlen = strlen(dirr->fid.cFileName);
	dirr->info.name = (char *)dirr->fid.cFileName;
	dirr->info.mode = 0;
	if (dirr->fid.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) dirr->info.mode |= BS_IFDIR;
	else dirr->info.mode |= BS_IFREG;
	if (dirr->fid.dwFileAttributes & FILE_ATTRIBUTE_READONLY) dirr->info.mode |= S_IREAD;
	else dirr->info.mode |= S_IREAD|S_IWRITE|S_IEXEC;
	tmp.HighPart = dirr->fid.nFileSizeHigh;
	tmp.LowPart = dirr->fid.nFileSizeLow;
	dirr->info.size = (boff_t)tmp.QuadPart;
	tmp.HighPart = dirr->fid.ftLastWriteTime.dwHighDateTime;
	tmp.LowPart = dirr->fid.ftLastWriteTime.dwLowDateTime;
	tmp.QuadPart -= INT64_C(116444736000000000);
	dirr->info.mtime = (btime_t)(tmp.QuadPart / 10000000);
	dirr->status++;
#elif defined(N64) || defined(__N64__)
	if (dirr->status < 0) return NULL;
	if (dirr->status > 0) {
		int ret = dir_findnext(dirr->root, &dirr->ldir);
		if (ret < 0) {
			dirr->status = -1;
			return NULL;
		}
	}
	dirr->info.namlen = strlen(dirr->ldir.d_name);
	dirr->info.name = dirr->ldir.d_name;
	dirr->info.mode = 0;
	if (dirr->ldir.d_type == DT_DIR) dirr->info.mode |= BS_IFDIR;
	else if (dirr->ldir.d_type == DT_REG) dirr->info.mode |= BS_IFREG;
	dirr->info.size = dirr->ldir.d_size < 0 ? 0 : (boff_t)dirr->ldir.d_size;
	dirr->info.mtime = 0;
	dirr->status++;
#else
	struct dirent *de;
	struct stat st;
	int fnlen;
	de = readdir(dirr->dir);
	if (de == NULL) {
		dirr->status = -1;
		return NULL;
	} else dirr->status++;
	dirr->info.namlen = strlen(de->d_name);
	dirr->info.name = de->d_name;
	dirr->info.mode = 0;
	dirr->info.size = 0;
	dirr->info.mtime = 0;
	fnlen = dirr->rootlen + 1 + dirr->info.namlen + 1;
	if (dirr->worklen < fnlen) {
		char *newwork = (char *)realloc(dirr->work, fnlen);
		if (!newwork) {
			dirr->status = -1;
			return NULL;
		}
		dirr->work = newwork;
		dirr->worklen = fnlen;
	}
	strcpy(&dirr->work[dirr->rootlen + 1], dirr->info.name);
	if (!stat(dirr->work, &st)) {
		dirr->info.mode = st.st_mode;
		dirr->info.size = st.st_size;
		dirr->info.mtime = st.st_mtime;
	}
#endif
	return &dirr->info;
}

int Bclosedir(BDIR *dir)
{
	BDIR_real *dirr = (BDIR_real*)dir;
#ifdef _MSC_VER
	FindClose(dirr->hfind);
#elif defined(N64) || defined(__N64__)
	free(dirr->root);
#else
	free(dirr->work);
	closedir(dirr->dir);
#endif
	free(dirr);
	return 0;
}'''
    path.write_text(s[:start] + block + s[end:])
    print(f"{path}: installed verified libdragon directory backend")

if __name__ == "__main__":
    if len(sys.argv) != 2: raise SystemExit("usage: patch-compat-dir-n64.py compat.c")
    patch(Path(sys.argv[1]))
