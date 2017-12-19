# J/psi to invisible decay

## Install v0.1 

> mkdir -p $HOME/bes/chic2invi

> cd $HOME/bes/chic2invi 

> git clone https://github.com/besiii/chic2invi.git v0.1 

## Initialize BOSS

BOSS verion 6.6.4.p03

This only need to be done for the first time after clone the code: 

> source init-boss.sh

## Setup 

> source setup.sh

## Build code

> ./build.sh 

## Submit jobs

> ./submit.sh

## For developers 

- Fork the code with your personal github ID. See [details](https://help.github.com/articles/fork-a-repo/)

> git clone git@github.com:yourid/jpsi2invi.git .

- Make your change, commit and push 

> git commit -a -m "Added feature A, B, C"
> git push

- Make a pull request to cepc. See [details](https://help.github.com/articles/using-pull-requests/)

## Some styles to follow 
- Minimize the number of main c++ files 
- Keep functions length less than one screen
- Seperate hard-coded cuts into script file
- Use pull-request mode on git 
- Document well the high-level bash file for work flow 


