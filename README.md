# concurrency_book_example

***
## Introduction
$\qquad$ this program give some C++ examples for the book: C++ Concurrency in Action 2nd Edition.pdf,include some examples from chapter1 to chapter9
***

## Some Advice
Concurrent programming is not simply adding mutual exclusion and adding threads, but also needs to pay attention to the memory model (the actual read and write order of data, and the visibility difference of data read and write order in a single thread and multiple threads), as well as the architecture of the CPU itself (involving cache synchronization and cache consistency between multiple threads), these two things determine concurrency safety and concurrency performance

some high star thread pool project on github: https://github.com/progschj/ThreadPool
