#include <iostream>

int main() {
  const char* code =
      "#include <iostream>%c%cint main() {%c    const char* code = %c%s%c;%c    printf(code, 10, 10, 10, 34, code, 34, 10, 10, 10);%c    return 0;%c}";
  printf(code, 10, 10, 10, 34, code, 34, 10, 10, 10);
  return 0;
}
