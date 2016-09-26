~~~
＊ CSci4061 F2016 Assignment 1
＊ login: zhou0745
＊ date: 10/04/2016
＊ name: Tiannan Zhou, full name2, full name3
＊ id: 5232494, id for second name, id for third name
~~~

**How to compile the program.**

~~~
make
~~~

You just need to use GNU makefile to compile this project.

    

**Who did what on the program**
* Tiannan Zhou
    * Completed the framework and algorithm part.
* Annelies Odermann
    * TBD
* Lidiya Dergacheva
    * TBD



**Syntax**

* Usage

 `./make4061 [options] [target]` Options can be combined together and only single target is allowed.

* Options

    1. `-f filename`: filename will be the name of the makefile, otherwise the default name ’makefile’ is assumed.

    2. `-n`: only display the commands it would run, but doesn't actually execute them.

    3. `-B`: Do not check timestamps for target and input and always re-compile.

* Targets

You can use only single target which was defined in Makefile. If you assign a nonexistent specified target, make4061 will return an error message.