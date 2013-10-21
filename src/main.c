#include "parser.h"

int main(void) {
  Mesh mesh;

  loadWavefront("cube.obj", &mesh);
  destroyWavefront(&mesh);
  
  return 0;
}
