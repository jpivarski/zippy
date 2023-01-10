#include <vector>

class superclass {
public:
  int i_am_super() { return -1; }
};

class class_2fields: public superclass {
public:
  class_2fields(int field1, int field2)
    : field1_(field1), field2_(field2) { }
private:
  int field1_;
  int field2_;
};

class class_3fields: public superclass {
public:
  class_3fields(int field1, int field2, int field3)
    : field1_(field1), field2_(field2), field3_(field3) { }
private:
  int field1_;
  int field2_;
  int field3_;
};

class class_4fields: public superclass {
public:
  class_4fields(int field1, int field2, int field3, int field4)
    : field1_(field1), field2_(field2), field3_(field3), field4_(field4) { }
private:
  int field1_;
  int field2_;
  int field3_;
  int field4_;
};

class class_5fields: public superclass {
public:
  class_5fields(int field1, int field2, int field3, int field4, int field5)
    : field1_(field1), field2_(field2), field3_(field3), field4_(field4), field5_(field5) { }
private:
  int field1_;
  int field2_;
  int field3_;
  int field4_;
  int field5_;
};

class class_6fields: public superclass {
public:
  class_6fields(int field1, int field2, int field3, int field4, int field5, int field6)
    : field1_(field1), field2_(field2), field3_(field3), field4_(field4), field5_(field5), field6_(field6) { }
private:
  int field1_;
  int field2_;
  int field3_;
  int field4_;
  int field5_;
  int field6_;
};

int main(int argc, char** argv) {
  int arg_value = 0;
  int arg_value1;
  int arg_value2;
  int arg_value3;
  int arg_value4;
  int arg_value5;
  int arg_value6;

  std::vector<superclass*> collection_2fields;
  std::vector<superclass*> collection_3fields;
  std::vector<superclass*> collection_4fields;
  std::vector<superclass*> collection_5fields;
  std::vector<superclass*> collection_6fields;

  for (int count_iterations = 0;  count_iterations < 1000;  count_iterations++) {
    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      arg_value1 = arg_value++;
      arg_value2 = arg_value++;
      collection_2fields.push_back(new class_2fields(arg_value1, arg_value2));
    }

    if (count_iterations != 0) {
      for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
        delete collection_6fields[count_instances];
      }
    }
    collection_6fields.clear();

    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      arg_value1 = arg_value++;
      arg_value2 = arg_value++;
      arg_value3 = arg_value++;
      collection_3fields.push_back(new class_3fields(arg_value1, arg_value2, arg_value3));
    }

    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      delete collection_2fields[count_instances];
    }
    collection_2fields.clear();

    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      arg_value1 = arg_value++;
      arg_value2 = arg_value++;
      arg_value3 = arg_value++;
      arg_value4 = arg_value++;
      collection_4fields.push_back(new class_4fields(arg_value1, arg_value2, arg_value3, arg_value4));
    }

    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      delete collection_3fields[count_instances];
    }
    collection_3fields.clear();

    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      arg_value1 = arg_value++;
      arg_value2 = arg_value++;
      arg_value3 = arg_value++;
      arg_value4 = arg_value++;
      arg_value5 = arg_value++;
      collection_5fields.push_back(new class_5fields(arg_value1, arg_value2, arg_value3, arg_value4, arg_value5));
    }

    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      delete collection_4fields[count_instances];
    }
    collection_4fields.clear();

    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      arg_value1 = arg_value++;
      arg_value2 = arg_value++;
      arg_value3 = arg_value++;
      arg_value4 = arg_value++;
      arg_value5 = arg_value++;
      arg_value6 = arg_value++;
      collection_6fields.push_back(new class_6fields(arg_value1, arg_value2, arg_value3, arg_value4, arg_value5, arg_value6));
    }

    for (int count_instances = 0;  count_instances < 10000;  count_instances++) {
      delete collection_5fields[count_instances];
    }
    collection_5fields.clear();

  }

  return 0;
}
