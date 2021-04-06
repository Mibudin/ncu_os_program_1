# Note of shell design


## TODOs

- [x] Redirection
    - [x] `<`
        - conti. followed `bash` (only last)
    - [ ] `<<`  <!-- Give up -->
    - [x] `>`
        - conti. followed `bash` (only last)
    - [x] `>>`
    - [x] `>&`
        - only `stderr`
    - [x] `>>&`
        - only `stderr`

- [x] Pipe

<!-- Give up -->
- [ ] `readline`

- [x] Builtin commands
    - [x] Exit
    - [x] Help
    - [x] Cd

<!-- Give up -->
- [ ] Config

- [x] Display exit code

- [x] Quotes parsing

- [x] Redundant blank (`\t`)

- [x] Multi-line input
    - [ ] `<<`  <!-- Give up -->

- [x] Multi-command

- [x] Background run

<!-- Give up -->
- [ ] Variables

<!-- Give up -->
- [ ] Alias
    - For first argument.

- [ ] Error control

<!-- Give up -->
- [ ] Escape character

- [x] `Const` in C

- [x] Colorized
    - [x] Prompt

- [ ] Report
