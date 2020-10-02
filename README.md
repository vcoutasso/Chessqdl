[![Build Status](https://travis-ci.com/vcoutasso/Chessqdl.svg?branch=develop)](https://travis-ci.com/vcoutasso/Chessqdl)

# ChessQDL

<h1 align="center">
    <img alt="Screenshot ChessQDL" src="https://vcoutasso.com/assets/images/chess.png" />
</h1>

Final project of the Data Structures 1 subject. Presented on 12/2019.

ChessQDL is a chess engine that uses a simple heuristic function to evaluate the current state of a given board and the minimax algorithm to choose a move within the tree of possible moves.

The engine was designed to allow for games of Player vs Engine and Player vs Player.

## Build instructions

Clone the repository:

```sh
$ git clone https://github.com/vcoutasso/Chessqdl
$ cd Chessqdl
```

Create the build directory:

```sh
$ mkdir build
$ cd build
```

Build Release version:

```sh
$ cmake ..
$ make
```

Run ChessQDL:

```sh
$ ./bin/ChessQDL
```

Build Debug version:

```sh
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
```

Note: Building in Debug mode will download Google Test from Github.

Run tests (Debug version only):

```sh
$ ctest -V
```

As an alternative to Unix Makefiles, other generators such as Ninja can be used:

```sh
$ cmake -G "Ninja" ..
$ ninja
```

Generate Doxygen files from code documentation:

```sh
$ cd docs
$ doxygen
```

## License

This software is free to use under the MIT License. See [this reference](https://choosealicense.com/licenses/mit/) for more information.
