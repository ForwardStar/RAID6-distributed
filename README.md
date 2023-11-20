## RAID6 Distributed File System
This is a RAID6 distributed file system.

Compile the codes:
```sh
make
```

It is recommended to install ``g++`` compiler with version ``>=9.3.0``.

Run the file system:
```sh
./main
```

Supported operations:
```
1. Create a file;
2. Delete a file;
3. View a file;
4. Modify a file (Feature under-developed).
```

You can verify RAID6 by:
```
1. Delete any one disk in "./filesystem";
2. Delete any two disks in "./filesystem".
```

A running example:
```
For a Galois field GF(2^n), a maximum number of 2^n storage disks is allowed.
Enter the degree 'n' of the Galois field (n < 32): 8
[INFO] [Mon Nov 20 23:49:33 2023] chunk_size = p.
[INFO] [Mon Nov 20 23:49:33 2023] Finding a monic irreducible polynomial (x = 2) of degree n.
[INFO] [Mon Nov 20 23:49:33 2023] The modulo irreducible polynomial is x ^ 8 + x ^ 2 + x + 1.
Enter the number of storage disks (1 - 256). Note that two extra disks would be used for checksum: 10
Enter the maximum number of chunks a disk can store: 10
[INFO] [Mon Nov 20 23:49:34 2023] The total storage of this file system is 0.000763MB.
[INFO] [Mon Nov 20 23:49:34 2023] Initializing RAID6 distributed file system...
[INFO] [Mon Nov 20 23:49:34 2023] Successfully initialize RAID6 distributed file system!
[INFO] [Mon Nov 20 23:49:34 2023] Checking whether there are missing disks.
[INFO] [Mon Nov 20 23:49:34 2023] No missing disks detected.
[INFO] [Mon Nov 20 23:49:34 2023] Current files: { }.
=====================
1. Create a file;
2. Delete a file;
3. View a file;
4. Modify a file;
5. Exit.
=====================
Perform an operation: 1
Enter the file name: a.txt
Enter the length of the file: 10
Enter the contents of the file (in binary format, e.g., '0001000101111'): 1010101010
[INFO] [Mon Nov 20 23:49:39 2023] Checking whether there are missing disks.
[INFO] [Mon Nov 20 23:49:39 2023] No missing disks detected.
[INFO] [Mon Nov 20 23:49:39 2023] Current files: { a.txt }.
=====================
1. Create a file;
2. Delete a file;
3. View a file;
4. Modify a file;
5. Exit.
=====================
Perform an operation: 1
Enter the file name: b.txt
Enter the length of the file: 10
Enter the contents of the file (in binary format, e.g., '0001000101111'): 0101010101
[INFO] [Mon Nov 20 23:49:49 2023] Checking whether there are missing disks.
[INFO] [Mon Nov 20 23:49:49 2023] No missing disks detected.
[INFO] [Mon Nov 20 23:49:49 2023] Current files: { b.txt a.txt }.
=====================
1. Create a file;
2. Delete a file;
3. View a file;
4. Modify a file;
5. Exit.
=====================
Perform an operation: 3
Enter the file name: a.txt
File content:
1010101010
[INFO] [Mon Nov 20 23:49:52 2023] Checking whether there are missing disks.
[INFO] [Mon Nov 20 23:49:52 2023] No missing disks detected.
[INFO] [Mon Nov 20 23:49:52 2023] Current files: { b.txt a.txt }.
=====================
1. Create a file;
2. Delete a file;
3. View a file;
4. Modify a file;
5. Exit.
=====================
Perform an operation: 5
```