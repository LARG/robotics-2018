# Git Branches 101

The UT Austin Villa codebase has a variety of branches for different purposes. The primary branch is `master` which can be found at `git@github.com:larg/spl.git`. The alias for this location is `origin`. Git refers to the `master` branch at `origin` with the shorthand `origin/master`.

## Creating your own branch

Let's say you've been making some code changes and you want to start tracking them at `origin`, but they're not quite ready for the `master` branch. The simplest way to do this is to create a new local branch and then push that local branch to `origin`. For example, let's say you've been working on a new line detector. You would create a new branch like this:

```bash
git checkout -b line-detector  # Create a new local branch
cd $NAO_HOME # Move to the repository root
git add . # Stage all files for a commit
git commit -m "Added a new line detector." # Commit staged files
git push -u origin line-detector  # Push the local branch to origin with the same name
```

Now you can go on another machine, pull down `line-detector`, and start where you left off.

## Merging into master

When you've fully verified your code and you're ready to add it to the `master` branch, you can do this by first pulling master into your branch, re-verifying your code, and then pulling your branch into master. Again, let's use the `line-detector` branch as an example:

1. Pull `master` into your branch:

    ```bash
    git checkout line-detector
    git pull origin master
    ```

2. If you're lucky, there will be no merge conflicts and you can skip to step 4. If there are merge conflicts, it's easiest to resolve these with meld. If you haven't already, start by configuring your git environment to use meld for merge conflicts:

    ```bash
    sudo apt-get install meld -y
    git conflig --global merge.tool meld
    ```
3. Once you have meld installed, you can use it to resolve merge conflicts by running `git mergetool`. Git will go through each file with merge conflicts and present you with three alternatives. 
  1. On the **left**, you'll see the `LOCAL` copy. This is the file as it exists in your current branch, in this case `line-detector`.
  2. On the **right**, you'll see the `REMOTE` copy. This is the file as it exists in the remote branch, in this case `origin/master`.
  3. In the **middle**, you'll see the `BASE` copy. This is the most recent common ancestor of `line-detector` and `origin/master` in the commit tree.

  Your task is to get the **middle** file to look the way you want, incorporating parallel changes that are found in both the `REMOTE` and `LOCAL` copies. Generally this process involves moving to differences between `LOCAL` and `REMOTE`, deciding which side is most up-to-date, and copying over the code from that side. Once you have merged all of the files, you are ready to commit the merge. Use `git add` to add any unstaged files, and then commit them with something like `git commit -m "Merged in origin/master"`.

4. Once you've merged `origin/master` into your branch you will want to test to make sure the merge was successful. First, build the entire branch with `$NAO_HOME/build/compile all`. When the build has completed, perform any tests you feel are necessary to ensure that your changes in the `line-detector` branch are still working and that the changes made to `origin/master` that you've incorporated are still working as well.

5. Now, push your `line-detector` changes to `origin` with `git push origin line-detector`. You can stop here if you're not quite ready to push to master, for instance if you have more features to add. It's good practice to complete these first 5 steps every so often just to stay in sync with the master branch.

6. When you're ready to push everything to `origin/master`, run the following commands:

    ```bash
    git checkout master # Switch to your local copy of origin/master
    git pull origin line-detector # Pull origin/line-detector into your local copy of master
    $NAO_HOME/build/compile all # Verify that the compile works for good measure
    git push origin master # Push into master
    ```

This completes the merge process.

## Diffs with meld

Before making a commit it is a good idea to diff your unstaged files against the current branch to make sure you only commit what you intend to. Running the `git diff` command will show you all changes, file by file. This feature can be enhanced with Meld by showing you differences through Meld's user interface. To use meld, first create a diff script at some accessible location, say `~/.gitdiff`:

```python
#!/usr/bin/python
import sys
import os
os.system('meld "%s" "%s"' % (sys.argv[2], sys.argv[5])) 
```

Now download and configure Git with Meld using this script:

```bash
chmod +x ~/.gitdiff
sudo apt-get install meld -y
git config --global diff.external ~/.gitdiff
```

## More resources

See the [GitHub Resources](https://help.github.com/articles/good-resources-for-learning-git-and-github/) page for more information.
