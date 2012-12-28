How to use git 
==============

### Forking the project

To make changes to this project, you should create your own fork, and
then issue pull requests when merges are desired. See:
https://help.github.com/articles/fork-a-repo for more info.

### Checkout your fork from Github
    git clone git@github.com:username/sdp

### See the changes files
    git status -s

### Add changed files
    git add -A

### Commit to local repository
    git commit -a -m "Your message"

### Commit to github

To commit code to the github first be sure your SSH public key is
generated in .ssh and added to your github account (see below).
Be sure to see: http://help.github.com/create-a-repo/ for more info.

    git push -u origin master


How to configure ssh public key  
------------------------------

Once you have a github account you must configure your public ssh key
for each computer you wish to use.

### Generate an ssh-rsa public key
    ssh-keygen

Next, copy the contents of id_rsa.pub and login to your github account.
Under account settings > SSH Keys click Add New SSH Key, paste, and 
done.

### Configure info
    
    git config --global user.name "Your Name"
    git config --global user.email your@email.com


Add/create a repository 
-----------------------

Next steps:
  mkdir sdp
  cd sdp
  git init
  touch README.md
  git add README.md
  git commit -m 'Initial commit.'
  git remote add origin git@github.com:username/sdp.git
  git push -u origin master
      
Existing Git Repo?
  cd existing_git_repo
  git remote add origin git@github.com:username/sdp.git
  git push -u origin master
