#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Trouver le premier caractère après le dernier slash.
  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Retourner le nom complété par des espaces s'il est trop court.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
  return buf;
}

// Fonction helper pour comparer le nom d'un fichier sans les espaces de padding.
int
namecmp(char *path, char *name)
{
  char base[DIRSIZ+1];
  int i;
  // Copier le nom formaté dans base.
  memmove(base, fmtname(path), DIRSIZ);
  base[DIRSIZ] = 0;
  // Supprimer les espaces de padding.
  for(i = 0; i < DIRSIZ; i++){
    if(base[i] == ' '){
      base[i] = 0;
      break;
    }
  }
  return strcmp(base, name);
}

// Recherche récursive dans le chemin "path" du fichier/dossier "name"
void
find(char *path, char *name, int *found)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    if(namecmp(path, name) == 0){
      // Si le nom correspond, afficher le résultat
      char *f = fmtname(path);
      // Pour éviter les espaces de padding, on copie et tronque
      char base[DIRSIZ+1];
      memmove(base, f, DIRSIZ);
      base[DIRSIZ] = 0;
      for(int i = 0; i < DIRSIZ; i++){
        if(base[i] == ' '){
          base[i] = 0;
          break;
        }
      }
      printf("NAME\tTYPE\tINO\tSIZE\n");
      printf("%s\t%d\t%d\t%d\n", base, st.type, st.ino, st.size);
      *found = 1;
    }
    break;

  case T_DIR:
    // Vérifier que le chemin ne soit pas trop long.
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      // Ignorer les entrées . et ..
      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      // Construire le chemin complet et lancer la recherche récursive
      find(buf, name, found);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int found = 0;

  if(argc != 3){
    printf("Usage : find <path> <nam>\n");
    exit(0);
  }
  // Vérifier que le premier argument est un chemin.
  if(argv[1][0] != '/' && argv[1][0] != '.'){
    printf("find : first argument must be a path, you gave %s\n", argv[1]);
    exit(0);
  }
  find(argv[1], argv[2], &found);
  if(!found){
    printf("find : %s not found!\n", argv[2]);
  }
  exit(0);
}
