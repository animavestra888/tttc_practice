/*
Вариант 4.
Добавть к статическим, локальным,глобальным объектам и параметрам функции
соответсвующие префиксы (static_, local_, global_, param_)
*/

int var1 = 0;

int foo(int a, int b) {
  static int var2 = 0;
  int var3 = 123;
  ++var2;
  return a + b + var1 + var2 + var3;
}

/*
Примерный вывод:

int global_var1 = 0;

int foo(int param_a, int param_b) {
  static int static_var2 = 0;
  int local_var3 = 123;
  ++static_var2;
  return param_a + param_b + global_var1 + static_var2 + local_var3;
}

*/

