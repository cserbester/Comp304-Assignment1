# Comp304-Assignment1
## My custom command count

The count command counts lines, words, and characters from a file or standard input.


## Usage

count
count filename.txt
count -l filename.txt
count -w filename.txt
count -c filename.txt

## Options

-l  : count only lines
-w  : count only words
-c  : count only characters

## Examples

count notes.txt
count -l notes.txt
cat notes.txt | count
