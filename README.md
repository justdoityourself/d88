# d88
C++ Data Research Library

## Introduction

[![D88](http://img.youtube.com/vi/oDPl95J3FXM/0.jpg)](https://www.youtube.com/watch?v=oDPl95J3FXM "D88")

## API Examples

```C++
#include "d88/api.hpp"

//...

//1024bit symmetric encryption:
auto key = "super secret string that will be converted into a key";
d88::api::default_encrypt("some_file", "encrypted_file", key);
d88::api::default_decrypt("encrypted_file", "decrypted_file", key);

//...

//128bytes parity per 4096 block:
d88::api::default_protect("some_file", "parity_db");
d88::api::default_recover("some_file", "parity_db");

```

## More Examples...

... todo custom application / configurations ...
