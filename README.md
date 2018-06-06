# What is the project

* a c interpreter supported x86_32 and x86_64 that interpretes itself.

* 64bit codes test successfully in ubuntu 16.04.

* warning free in the project.

* There are also bugs in the interpreter like the computation with `&&` and `Sizeof` function.

# Contents

`c-interpreter32` c interpreter implement in x86_32.

`c-interpreter64` c interpreter implement in x86_64.

`reference` three forks in the project, most codes are based on them.

`steps` tutorial step by step.

`README.md`

`LICENSE`

# How to Run the Code

### 32bit Machine

```
//nodebug
gcc -o ic32 ic32.c
./ic32 hello.c

//debug
gcc -o ic32-debug ic32-debug.c
./ic32-debug hello.c
```
### 64bit Machine

```
//nodebug
gcc -o ic64 ic64.c
./ic64 hello.c

//debug
gcc -o ic64-debug ic64-debug.c
./ic64-debug hello.c
```

# About

Behand informations are important ones to write the interpreter in [write-a-C-interpreter](https://github.com/lotabout/write-a-C-interpreter)

I just move them out in the place.

This project is inspired by [c4](https://github.com/rswier/c4) and is largely
based on it.

However, I rewrited them all to make it more understable and help myself to
understand it.

Despite the complexity we saw in books about compiler design, writing one is
not that hard. You don't need that much theory though they will help for
better understanding the logic behind the code.

Also I write a series of article about how this compiler is built under directory `tutorial/en`.

There is also a chinese version in my blog.

1. [手把手教你构建 C 语言编译器（0）——前言](http://lotabout.me/2015/write-a-C-interpreter-0/)
2. [手把手教你构建 C 语言编译器（1）——设计](http://lotabout.me/2015/write-a-C-interpreter-1/)
3. [手把手教你构建 C 语言编译器（2）——虚拟机](http://lotabout.me/2015/write-a-C-interpreter-2/)
4. [手把手教你构建 C 语言编译器（3）——词法分析器](http://lotabout.me/2015/write-a-C-interpreter-3/)
4. [手把手教你构建 C 语言编译器（4）——递归下降](http://lotabout.me/2016/write-a-C-interpreter-4/)
5. [手把手教你构建 C 语言编译器（5）——变量定义](http://lotabout.me/2016/write-a-C-interpreter-5/)
6. [手把手教你构建 C 语言编译器（6）——函数定义](http://lotabout.me/2016/write-a-C-interpreter-6/)
7. [手把手教你构建 C 语言编译器（7）——语句](http://lotabout.me/2016/write-a-C-interpreter-7/)
8. [手把手教你构建 C 语言编译器（8）——表达式](http://lotabout.me/2016/write-a-C-interpreter-8/)
0. [手把手教你构建 C 语言编译器（9）——总结](http://lotabout.me/2016/write-a-C-interpreter-9/)

# Resources

Further Reading:

- [Let's Build a Compiler](http://compilers.iecc.com/crenshaw/): An excellent
    starting material for building compiler.


Forks:

- [A fork that is the source about c4](https://github.com/rswier/c4)

- [A fork that implement more for c4.c](https://github.com/lotabout/write-a-C-interpreter)

- [A fork that implement debugger for xc.c](https://github.com/descent/write-a-C-interpreter)


# Licence

The original code is licenced with GPL2, so this code will use the same
licence.
