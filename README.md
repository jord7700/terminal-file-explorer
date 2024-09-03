# README
A terminal GUI to navigate file system with less typing and `cd && ls`-ing

`Ctrl-C` to exit to terminal in current selected directory

# Build
```
cmake -S . -B build
cmake --build build
./build/terminal-file-explorer
```

# Shortcuts
Backspace - Go up a directory

Enter - Enter a directory

Home - Jump to top of list

End - Jump to bottom of list

h - Toggle hidden files and directories

d - Mix directories and files (Default filters files to bottom of list)


# TODO

<ul>
    <li>Open to default terminal instead of bash</li>
    <li>Tab complete manual paths</li>
    <li>Hide files option</li>
    <li>Manual or help page</li>
    <li>Scrollbar on long lists</li>
</ul>