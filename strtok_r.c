int main( int argc, char *argv[]) 
{
  FILE *pFile;
  char line[100] = {0x00, };
  char record[100][100] = {0x00, };
  char full_path[30];
  char fname[100];
  int i = 0;
  char *saveptr1;
  char *saveptr2;
  
  scanf("%s", fname);
  
  sprintf( full_path, ".//BIGFILE//%s", fname);
  
  pFile = fopen( full_path, "r");
  
  if (pFile == NULL)
    perror("file open error");
  
  while( !feof(pFile) )
  {
    fgets ( line, 100, pFile );
    
    char *ptr = strtok_r(line, "#", &saveptr1 );
    
    while( ptr != NULL )
    {
    
      char * subptr = strtok_r( ptr, ",",  &saveptr2 );
      while( ptr != NULL )
      {
        // do something
        printf(" sub : %s\n", subptr);
        subptr = strtok_r(NULL, ",",   &saveptr2 );
      }
      
        ptr = strtok_r(NULL, "#",   &saveptr1 );
    }
  
    fclose(pFile);
  
  return 0;
  
  }
    
    

}