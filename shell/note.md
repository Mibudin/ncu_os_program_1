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

- [ ] Config

- [ ] Display exit code

- [x] Quotes parsing

- [x] Redundant blank (`\t`)

- [x] Multi-line input
    - [ ] `<<`

- [x] Multi-command

- [x] Background run

<!-- Give up -->
- [ ] Variables

- [ ] Alias
    - For first argument.

- [ ] Error control

<!-- Give up -->
- [ ] Escape character

- [ ] `Const` in C

- [ ] Colorized
