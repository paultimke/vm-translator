This is a Virtual Machine translator / interpreter written in C, which translates bytecode into assembly code for the Hack computer from the Nand 2 Tetris course:\
https://www.coursera.org/learn/build-a-computer/

# Building the project
`git clone https://github.com/paultimke/hack-compiler`\
`cd hack-compiler`\
`git submodule update --init --recursive`\
`./build.sh release`

You can compile programs with:\
`jackc <Path to file.jack>`

Or translate VM bytecode with:\
`vm-translator <Path to file.vm>`
