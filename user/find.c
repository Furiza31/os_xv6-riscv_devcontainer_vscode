#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// Extrait le nom de base d'un chemin
char*
basename(char *path)
{
  char *p;
  // On se positionne à la fin de la chaîne
  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  return p;
}

// Fonction de recherche récursive
void
find(char *path, char *target)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  // Ouvre le chemin passé en paramètre
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: impossible d'ouvrir %s\n", path);
    return;
  }
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: impossible de stat %s\n", path);
    close(fd);
    return;
  }

  // Si ce n'est pas un répertoire, on vérifie si le nom correspond
  if(st.type != T_DIR){
    if(strcmp(basename(path), target) == 0)
      printf("%s\n", path);
    close(fd);
    return;
  }

  // c'est un répertoire : on prépare le buffer pour les chemins
  if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
    printf("find: chemin trop long\n");
    close(fd);
    return;
  }
  strcpy(buf, path);
  p = buf + strlen(buf);
  *p++ = '/';

  // Parcourt le répertoire
  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    // ignorer les entrées vides
    if(de.inum == 0)
      continue;

    // Copie le nom de l'entrée dans le buffer (attention, de.name est de taille fixe)
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0; // termine la chaîne

    // Élimine les espaces ajoutés pour remplir DIRSIZ
    for(int i = strlen(p)-1; i >= 0 && p[i]==' '; i--)
      p[i] = 0;

    // Évite de parcourir "." et ".."
    if(strcmp(p, ".") == 0 || strcmp(p, "..") == 0)
      continue;

    // Récupère le statut du fichier/dossier
    if(stat(buf, &st) < 0){
      fprintf(2, "find: impossible de stat %s\n", buf);
      continue;
    }

    // Si le nom correspond, on affiche le chemin complet
    if(strcmp(basename(buf), target) == 0)
      printf("%s\n", buf);

    // Si c'est un répertoire, on appelle récursivement find
    if(st.type == T_DIR)
      find(buf, target);
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(2, "Usage: find <chemin> <nom>\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}
