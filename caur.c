//     ___ __ _ _   _ _ __
//    / __/ _` | | | | '__|   caur:
//   | (_| (_| | |_| | |      A simple AUR helper written in C.
//    \___\__,_|\__,_|_|
// Written by Madison Lynch <madi@mxdi.xyz>
// Source: https://github.com/mxdixyz/caur
// License: MIT (See LICENSE file)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <git2.h>
#include <sys/stat.h>
#include <unistd.h>

char pkgPath[1024];
struct stat st = {0};

/** @brief Prints help info
 *
 *  Prints help info to stdout.
 *
 *  @param name: Program name.
 *  @return Void.
 */
void
printHelp(const char *name) {
	printf("Usage: %s <operation> [targets]\n", name);
	printf("Operations:\n");
	printf("-S\t\tInstall package\n");
	printf("-R\t\tRemove package\n");
	printf("-C\t\tClear package cache\n");
	printf("-h, --help\tPrints this menu\n");

	exit(EXIT_SUCCESS);
}

/** @brief Install package
 *
 *  Clones Git repository for specified
 *  AUR package, and installs with makepkg.
 *
 *  @param pkg: Name of package to install.
 *  @return Void.
 */
void
installPKG(const char *pkg) {
	strcat(pkgPath, pkg);
	if(stat(pkgPath, &st) == -1) {
		git_libgit2_init();
		mkdir(pkgPath, 0700);

		char link[256];
		snprintf(link, sizeof(link), "https://aur.archlinux.org/%s.git", pkg);

		git_repository *repo;
		git_clone(&repo, link, pkgPath, NULL);
		git_repository_free(repo);
	}

	char pkgBuildPath[1280];
	snprintf(pkgBuildPath, sizeof(pkgBuildPath), "%s/PKGBUILD", pkgPath);

	char cmd[1031];
	if(access(pkgBuildPath, F_OK) != 0) {
		fprintf(stderr, "target not found: %s\n", pkg);
		snprintf(cmd, sizeof(cmd), "rm -rf %s", pkgPath);
		system(cmd);
		exit(EXIT_FAILURE);
	}

	char conf, buff[256], less[1285];
	printf("Check contents of PKGBUILD? [Y/n] ");
	fgets(buff, sizeof(buff), stdin);
	conf = buff[0];
	if(conf == 'Y' || conf == 'y' || conf == '\n') {
		snprintf(less, sizeof(less), "less %s", pkgBuildPath);
		system(less);
	} else if(conf != 'N' && conf !='n') {
		fprintf(stderr, "Invalid input\n");
		exit(EXIT_FAILURE);
	}

	printf("Proceed with installation? [Y/n] ");
	fgets(buff, sizeof(buff), stdin);
	conf = buff[0];
	if(conf == 'Y' || conf == 'y' || conf == '\n') {
		char cwd[256];
		getcwd(cwd, sizeof(cwd));

		chdir(pkgPath);
		system("makepkg -si");

		chdir(cwd);
	} else
		return;

	git_libgit2_shutdown();
}

/** @brief Uninstall package
 *
 *  Uninstalls package with Pacman.
 *
 *  @param pkg: Name of package to remove.
 *  @return Void.
 */
void
removePKG(const char *pkg) {
	char cmd[1050];
	if(access("/usr/bin/sudo", F_OK) == 0)
		snprintf(cmd, sizeof(cmd), "sudo pacman -Rncs %s", pkg);
	else if(access("/usr/bin/doas", F_OK) == 0)
		snprintf(cmd, sizeof(cmd), "doas pacman -Rncs %s", pkg);
	else
		snprintf(cmd, sizeof(cmd), "su root -c \"pacman -Rncs %s\"", pkg);
	system(cmd);
}

/** @brief Clear cache
 *
 *  Clears package cache stored in
 *  "~/.cache/caur/".
 *
 *  "NULL" input clears all package cache.
 *
 *  @param pkg: Name of package cache to remove.
 *  @return Void.
 */
void
clearCache(const char *pkg) {
	char cmd[2055];
	if(pkg == NULL)
		snprintf(cmd, sizeof(cmd), "rm -rf %s/*", pkgPath);
	else {
		strcat(pkgPath, pkg);
		if(stat(pkgPath, &st) == -1) {
			fprintf(stderr, "target not found: %s\n", pkg);
			exit(EXIT_FAILURE);
		} else {
			snprintf(pkgPath, sizeof(pkgPath), "%s/.cache/caur/", getenv("HOME"));
			snprintf(cmd, sizeof(cmd), "rm -rf %s/%s", pkgPath, pkg);
		}
	}
	system(cmd);
}

int
main(int argc, char **argv) {
	snprintf(pkgPath, sizeof(pkgPath), "%s/.cache/caur/", getenv("HOME"));
	if(stat(pkgPath, &st) == -1)
		mkdir(pkgPath, 0700);

	if(argc < 2 || argv[1][0] != '-') {
		fprintf(stderr, "%s: no operation specified (use -h for help)\n", argv[0]);
		return EXIT_FAILURE;
	}

	switch(argv[1][1]) {
		case 'S':
			if(argc < 3) {
				fprintf(stderr, "%s: no targets specified (use -h for help)\n", argv[0]);
				return EXIT_FAILURE;
			}
				
			for(int i=2; i<argc; i++) {
				installPKG(argv[i]);
				snprintf(pkgPath, sizeof(pkgPath), "%s/.cache/caur/", getenv("HOME"));
			}
			break;

		case 'R':
			if(argc < 3) {
				fprintf(stderr, "%s: no targets specified (use -h for help)\n", argv[0]);
				return EXIT_FAILURE;
			}

			for(int i=2; i<argc; i++)
				removePKG(argv[i]);
			break;

		case 'C':
			if(argc < 3)
				clearCache(NULL);
			else
				for(int i=2; i<argc; i++)
					clearCache(argv[i]);
			break;

		case 'h':
			printHelp(argv[0]);
			break;

		default:
			if(strcmp(argv[1], "--help") == 0)
				printHelp(argv[0]);
			else {
				fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], argv[1]);
				return EXIT_FAILURE;
			}
				
	}

	return EXIT_SUCCESS;
}