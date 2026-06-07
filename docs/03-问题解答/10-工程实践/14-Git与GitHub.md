# Git与GitHub实战指南
> 📖 相关章节：[编码规范](../../04-工程实践/00-编码规范.md)、[设计模式](../../04-工程实践/03-设计模式.md)、[单元测试](../../04-工程实践/06-单元测试.md)、[代码审查](../../04-工程实践/08-代码审查.md)

## 1. Git是什么

### 1. 版本控制系统

Git是一个**分布式版本控制系统**，用于记录文件的每次变更，让你可以随时回溯到任意历史版本。

```text
没有版本控制的世界：
  report_v1.doc
  report_v2.doc
  report_v3_最终版.doc
  report_v4_真的最终版.doc
  report_v5_打死不改版.doc
  report_v6_老板说要改版.doc

使用Git的世界：
  report.doc  ← 所有历史版本都在 .git 里，一个文件搞定
```

### 2. 类比理解：游戏存档系统

```text
游戏存档系统              Git版本控制
─────────────           ─────────────
存档点（Save）     ←→    提交（commit）
读档（Load）       ←→    检出（checkout）
分支剧情           ←→    分支（branch）
合并结局           ←→    合并（merge）
回退到之前存档     ←→    回退（reset/revert）
```

你可以随时存档（commit），随时读档（checkout），还能回到之前的任意存档。

### 3. 分布式 vs 集中式

```text
集中式版本控制（SVN）：
  ┌─────────┐
  │ 中央服务器 │ ← 唯一完整仓库，断网无法工作
  └────┬────┘
       │
  ┌────┴────┐
  │  开发者  │ ← 只有工作副本，没有完整历史
  └─────────┘

分布式版本控制（Git）：
  ┌─────────┐
  │ 远程仓库  │ ← GitHub/GitLab
  └────┬────┘
       │
  ┌────┴────────────────┐
  │                     │
┌─┴───┐             ┌──┴──┐
│开发者A│             │开发者B│
│完整仓库│             │完整仓库│  ← 每人都有完整历史，断网也能工作
└──────┘             └─────┘
```

| 特性 | Git（分布式） | SVN（集中式） |
|------|-------------|-------------|
| 离线工作 | 支持 | 不支持 |
| 速度 | 快（本地操作） | 慢（依赖网络） |
| 分支 | 轻量级 | 重量级 |
| 安全性 | 每人完整备份 | 依赖中央服务器 |

## 2. 安装与初始配置

### 1. 安装Git

**Windows：**

```bash
# 下载安装 Git for Windows
# 官网：https://git-scm.com/download/win
# 安装后打开 Git Bash 验证
git --version
# 输出：git version 2.43.0.windows.1
```

**Linux（Ubuntu/Debian）：**

```bash
sudo apt update
sudo apt install git

git --version
# 输出：git version 2.43.0
```

**macOS：**

```bash
# 方式一：Xcode命令行工具
xcode-select --install

# 方式二：Homebrew
brew install git

git --version
# 输出：git version 2.43.0
```

### 2. 必须配置

```bash
# 配置用户名（会出现在每次提交记录中）
git config --global user.name "张三"

# 配置邮箱（需要和GitHub注册邮箱一致）
git config --global user.email "zhangsan@example.com"

# 配置默认编辑器
git config --global core.editor "code --wait"
# 其他选择：
# vim:   git config --global core.editor "vim"
# nano:  git config --global core.editor "nano"

# 配置默认分支名为 main
git config --global init.defaultBranch main

# 查看所有配置
git config --list
# 输出：
# user.name=张三
# user.email=zhangsan@example.com
# core.editor=code --wait
# init.defaultbranch=main

# 查看某一项配置
git config user.name
# 输出：张三
```

配置的三个层级：

```text
--system   → /etc/gitconfig        → 所有用户生效
--global   → ~/.gitconfig          → 当前用户所有仓库生效
--local    → .git/config           → 仅当前仓库生效（默认）

优先级：local > global > system
```

### 3. SSH密钥配置

```bash
# 第一步：生成SSH密钥
ssh-keygen -t ed25519 -C "zhangsan@example.com"
# 输出：
# Generating public/private ed25519 key pair.
# Enter file in which to save the key (/c/Users/张三/.ssh/id_ed25519):
# Enter passphrase (empty for no passphrase):
# Enter same passphrase again:
# Your identification has been saved in /c/Users/张三/.ssh/id_ed25519
# Your public key has been saved in /c/Users/张三/.ssh/id_ed25519.pub

# 第二步：查看公钥内容
cat ~/.ssh/id_ed25519.pub
# 输出：
# ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAI... zhangsan@example.com

# 第三步：将公钥添加到GitHub
# GitHub → Settings → SSH and GPG keys → New SSH key
# 粘贴公钥内容，保存

# 第四步：测试连接
ssh -T git@github.com
# 输出：
# Hi zhangsan! You've successfully authenticated, but GitHub does not provide shell access.

# 第五步：配置SSH代理（可选，避免每次输入密码）
eval "$(ssh-agent -s)"
# 输出：Agent pid 12345

ssh-add ~/.ssh/id_ed25519
# 输出：Identity added: /c/Users/张三/.ssh/id_ed25519 (zhangsan@example.com)
```

## 3. Git核心概念

### 1. 工作区/暂存区/仓库 三层结构

```text
                    git add              git commit
工作区（Working） ──────────→ 暂存区（Staging） ──────────→ 仓库（Repository）
   │                           │                            │
   │  你编辑文件的地方           │  下次提交的快照              │  所有提交的历史
   │                           │                            │
   │  ← git checkout           │  ← git reset               │
   │      （丢弃修改）           │      （取消暂存）            │
```

实际目录结构：

```text
my-project/           ← 工作区（你看到的文件）
├── .git/             ← 仓库（Git内部数据库）
│   ├── objects/      ← 所有文件快照
│   ├── refs/         ← 分支和标签指针
│   ├── HEAD          ← 当前分支指针
│   └── index         ← 暂存区
├── src/
│   └── main.cpp
└── README.md
```

文件的三种状态：

```text
已修改（Modified）   → 文件已更改但未暂存
已暂存（Staged）     → 文件已标记将包含在下次提交中
已提交（Committed）   → 文件已安全保存在本地仓库中
```

### 2. commit/branch/merge/rebase 图解

**commit（提交）：**

```text
每次commit生成一个快照，通过父指针串联：

  A ← B ← C ← D
              ↑
            HEAD
            main

每个节点包含：
  - 唯一的SHA-1哈希值
  - 作者信息
  - 提交信息
  - 指向父提交的指针
  - 项目快照
```

**branch（分支）：**

```text
分支只是指向某个commit的可移动指针：

创建分支前：
  A ← B ← C
              ↑
            main (HEAD)

创建并切换到feature分支：
  A ← B ← C
              ↑
            main
            feature (HEAD)

在feature上继续提交：
  A ← B ← C ← D ← E
              ↑       ↑
            main    feature (HEAD)
```

**merge（合并）：**

```text
三方合并（创建合并提交）：

合并前：
  A ← B ← C ← D
       ↑       ↑
     main    feature (HEAD)

合并后：
  A ← B ← C ← D ← M
       ↑       ↑   ↑
     main    feature HEAD
                  (main)

M 是合并提交，有两个父提交 C 和 D
```

**rebase（变基）：**

```text
变基（重写提交历史）：

变基前：
  A ← B ← C ← D
       ↑       ↑
     main    feature

执行 git rebase main（在feature分支上）：
  A ← B ← C ← D'
                ↑  ↑
              main feature

D' 是 D 的重写版本，历史变成线性
```

merge vs rebase：

```text
merge：保留完整历史，产生合并提交，历史有分叉
rebase：线性历史，更干净，但改写了提交历史

        merge结果：          rebase结果：
          C ← D              C ← D'
         ↗     ↘            ↑     ↑
  A ← B         M           A ← B
         ↘     ↗                  ↑
          E ← F                   E ← F'
```

### 3. HEAD指针的概念

```text
HEAD 是一个指针，指向当前所在的提交或分支：

正常状态（指向分支）：
  A ← B ← C
              ↑
            main ← HEAD

分离头指针状态（指向具体提交）：
  A ← B ← C
       ↑
      HEAD  ← 直接指向提交，不指向分支

查看HEAD指向：
  cat .git/HEAD
  # 输出：ref: refs/heads/main  （指向分支）
  # 或：4a2b3c...              （分离头指针）
```

## 4. 日常操作完全指南

### 1. git init / clone — 创建/克隆仓库

**初始化新仓库：**

```bash
# 创建新目录并初始化
mkdir my-project
cd my-project
git init
# 输出：
# Initialized empty Git repository in /path/to/my-project/.git/

# 在已有目录中初始化
cd existing-project
git init
# 输出：
# Initialized empty Git repository in /path/to/existing-project/.git/

# 初始化后查看状态
git status
# 输出：
# On branch main
# No commits yet
# nothing to commit
```

**克隆远程仓库：**

```bash
# HTTPS方式克隆
git clone https://github.com/user/repo.git
# 输出：
# Cloning into 'repo'...
# remote: Enumerating objects: 150, done.
# remote: Counting objects: 100% (150/150), done.
# remote: Compressing objects: 100% (80/80), done.
# Receiving objects: 100% (150/150), 2.50 MiB | 1.20 MiB/s, done.
# Resolving deltas: 100% (60/60), done.

# SSH方式克隆
git clone git@github.com:user/repo.git

# 克隆到指定目录
git clone https://github.com/user/repo.git my-folder

# 克隆指定分支
git clone -b develop https://github.com/user/repo.git

# 浅克隆（只获取最近的历史，节省时间和空间）
git clone --depth 1 https://github.com/user/repo.git
# 输出：
# Cloning into 'repo'...
# remote: Enumerating objects: 30, done.
# ...
```

### 2. git add / commit — 暂存和提交

**暂存文件：**

```bash
# 暂存单个文件
git add main.cpp
# 无输出表示成功

# 暂存多个文件
git add main.cpp utils.cpp config.h

# 暂存所有修改的文件
git add .

# 暂存所有已跟踪的修改文件（不包括新文件）
git add -u

# 交互式暂存（逐块选择）
git add -p main.cpp
# 输出：
# diff --git a/main.cpp b/main.cpp
# index 1a2b3c4..5d6e7f8 100644
# --- a/main.cpp
# +++ b/main.cpp
# @@ -1,3 +1,4 @@
#  #include <iostream>
# +#include <vector>
#  int main() {
#      return 0;
# Stage this hunk [y,n,q,a,d,/,s,e,?]?
```

**提交：**

```bash
# 提交暂存区的内容
git commit -m "feat: 添加用户登录功能"
# 输出：
# [main 3a4b5c6] feat: 添加用户登录功能
#  2 files changed, 50 insertions(+), 10 deletions(-)
#  create mode 100644 src/login.cpp

# 跳过暂存，直接提交所有已跟踪文件的修改
git commit -a -m "fix: 修复内存泄漏问题"
# 输出：
# [main 7d8e9f0] fix: 修复内存泄漏问题
#  1 file changed, 3 insertions(+), 1 deletion(-)

# 打开编辑器编写详细提交信息
git commit
# 编辑器中会显示：
#
# 请输入提交信息以解释此变更...
# 以 '#' 开头的行将被忽略，空提交将被丢弃
#
# 位于分支 main
# 要提交的变更：
#       新文件：   src/login.cpp
#       修改：     src/main.cpp
```

提交信息规范（Conventional Commits）：

```text
<类型>(<范围>): <描述>

类型：
  feat     → 新功能
  fix      → 修复Bug
  docs     → 文档变更
  style    → 代码格式（不影响逻辑）
  refactor → 重构（不是新功能也不是修复）
  perf     → 性能优化
  test     → 添加测试
  chore    → 构建过程或辅助工具变动

示例：
  feat(auth): 添加JWT令牌验证
  fix(memory): 修复智能指针循环引用
  docs(readme): 更新安装说明
  refactor(utils): 提取公共函数到工具模块
```

### 3. git status / log / diff — 查看状态

**查看状态：**

```bash
git status
# 输出：
# On branch main
# Changes to be committed:
#   (use "git restore --staged <file>..." to unstage)
#         new file:   src/login.cpp
#
# Changes not staged for commit:
#   (use "git add <file>..." to update what will be committed)
#   (use "git restore <file>..." to discard changes in working directory)
#         modified:   src/main.cpp
#
# Untracked files:
#   (use "git add <file>..." to include in what will be committed)
#         src/config.h

# 简洁模式
git status -s
# 输出：
# A  src/login.cpp      ← A=已暂存的新文件
#  M src/main.cpp       ← M=已修改未暂存（空格+M）
# ?? src/config.h       ← ??=未跟踪
```

**查看日志：**

```bash
# 完整日志
git log
# 输出：
# commit 3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b (HEAD -> main)
# Author: 张三 <zhangsan@example.com>
# Date:   Thu May 21 10:30:00 2026 +0800
#
#     feat: 添加用户登录功能
#
# commit 1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b
# Author: 张三 <zhangsan@example.com>
# Date:   Thu May 21 09:00:00 2026 +0800
#
#     init: 初始化项目

# 单行显示
git log --oneline
# 输出：
# 3a4b5c6 feat: 添加用户登录功能
# 1a2b3c4 init: 初始化项目

# 图形化显示分支
git log --oneline --graph --all
# 输出：
# * 3a4b5c6 (HEAD -> main) feat: 添加用户登录功能
# * 1a2b3c4 init: 初始化项目

# 显示最近3条
git log -3
# 输出：
# commit 3a4b5c6...

# 显示每次提交的文件变更统计
git log --stat
# 输出：
# commit 3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b
# Author: 张三 <zhangsan@example.com>
# Date:   Thu May 21 10:30:00 2026 +0800
#
#     feat: 添加用户登录功能
#
#  src/login.cpp | 30 ++++++++++++++++++++++++++++++
#  src/main.cpp  |  5 ++++-
#  2 files changed, 34 insertions(+), 1 deletion(-)

# 按作者筛选
git log --author="张三"

# 按关键词搜索提交信息
git log --grep="登录"

# 按时间范围
git log --since="2026-01-01" --until="2026-05-21"

# 查看某个文件的修改历史
git log -p src/main.cpp
```

**查看差异：**

```bash
# 工作区 vs 暂存区（未暂存的修改）
git diff
# 输出：
# diff --git a/src/main.cpp b/src/main.cpp
# index 1a2b3c4..5d6e7f8 100644
# --- a/src/main.cpp
# +++ b/src/main.cpp
# @@ -1,4 +1,5 @@
#  #include <iostream>
# +#include <vector>
#  int main() {
# -     return 0;
# +     std::vector<int> v;
# +     return 0;
#  }

# 暂存区 vs 仓库（已暂存但未提交的修改）
git diff --staged
# 输出：
# diff --git a/src/login.cpp b/src/login.cpp
# new file mode 100644
# index 0000000..3a4b5c6
# --- /dev/null
# +++ b/src/login.cpp
# @@ -0,0 +1,30 @@
# +#include "login.h"
# +bool login(const std::string& user) {
# +    return true;
# +}

# 比较两个提交
git diff 1a2b3c4 3a4b5c6

# 比较两个分支
git diff main feature

# 只显示文件名和变更统计
git diff --stat
# 输出：
#  src/main.cpp | 3 ++-
#  1 file changed, 2 insertions(+), 1 deletion(-)

# 查看某个文件在两次提交间的差异
git diff 1a2b3c4 3a4b5c6 -- src/main.cpp
```

### 4. git push / pull — 推送和拉取

**推送：**

```bash
# 首次推送并设置上游分支
git push -u origin main
# 输出：
# Enumerating objects: 7, done.
# Counting objects: 100% (7/7), done.
# Delta compression using up to 8 threads
# Compressing objects: 100% (4/4), done.
# Writing objects: 100% (7/7), 1.20 KiB | 1.20 MiB/s, done.
# To https://github.com/user/repo.git
#  * [new branch]      main -> main
# Branch 'main' set up to track remote branch 'main' from 'origin'.

# 后续推送（已设置上游）
git push
# 输出：
# Everything up-to-date  （没有新提交时）
# 或：
# Enumerating objects: 5, done.
# ...
# To https://github.com/user/repo.git
#    1a2b3c4..3a4b5c6  main -> main

# 推送所有分支
git push --all

# 推送标签
git push origin v1.0.0

# 推送所有标签
git push --tags

# 强制推送（危险！会覆盖远程历史）
git push -f origin main
# 输出：
# To https://github.com/user/repo.git
# + 1a2b3c4...3a4b5c6  main -> main (forced update)
```

**拉取：**

```bash
# 拉取并合并（相当于 fetch + merge）
git pull
# 输出：
# remote: Enumerating objects: 5, done.
# remote: Counting objects: 100% (5/5), done.
# remote: Compressing objects: 100% (3/3), done.
# Unpacking objects: 100% (3/3), done.
# From https://github.com/user/repo
#    1a2b3c4..7d8e9f0  main        -> origin/main
# Updating 1a2b3c4..7d8e9f0
# Fast-forward
#  src/utils.cpp | 10 ++++++++++
#  1 file changed, 10 insertions(+)

# 使用rebase方式拉取（保持线性历史）
git pull --rebase
# 输出：
# remote: Enumerating objects: 5, done.
# ...
# From https://github.com/user/repo
#    1a2b3c4..7d8e9f0  main        -> origin/main
# Rebasing (1/1)
# Successfully rebased and updated refs/heads/main.

# 只拉取不合并（查看远程更新内容）
git fetch
# 输出：
# remote: Enumerating objects: 5, done.
# remote: Counting objects: 100% (5/5), done.
# ...
# From https://github.com/user/repo
#  * [new branch]      feature    -> origin/feature
#    1a2b3c4..7d8e9f0  main       -> origin/main

# 查看fetch后的差异
git log HEAD..origin/main
git diff HEAD..origin/main

# 确认后再合并
git merge origin/main
```

### 5. git branch / checkout / switch — 分支操作

**分支管理：**

```bash
# 查看所有本地分支
git branch
# 输出：
# * main        ← * 表示当前分支
#   develop
#   feature-login

# 查看所有分支（包括远程）
git branch -a
# 输出：
# * main
#   develop
#   feature-login
#   remotes/origin/main
#   remotes/origin/develop
#   remotes/origin/feature-api

# 创建新分支
git branch feature-register
# 无输出表示成功

# 创建并切换到新分支
git checkout -b feature-register
# 输出：
# Switched to a new branch 'feature-register'

# 或使用新语法（Git 2.23+）
git switch -c feature-register
# 输出：
# Switched to a new branch 'feature-register'

# 切换分支
git checkout main
# 输出：
# Switched to branch 'main'
# Your branch is up to date with 'origin/main'.

# 或使用新语法
git switch main
# 输出：
# Switched to branch 'main'

# 删除已合并的分支
git branch -d feature-register
# 输出：
# Deleted branch feature-register (was 3a4b5c6).

# 强制删除未合并的分支
git branch -D feature-register
# 输出：
# Deleted branch feature-register (was 3a4b5c6).

# 重命名分支
git branch -m old-name new-name

# 查看分支的最后一次提交
git branch -v
# 输出：
# * main            3a4b5c6 feat: 添加用户登录功能
#   develop         7d8e9f0 refactor: 重构配置模块
#   feature-login   1a2b3c4 WIP: 登录页面
```

### 6. git merge / rebase — 合并和变基

**合并（merge）：**

```bash
# 切换到目标分支
git checkout main

# 合并feature分支
git merge feature-login
# 快进合并（没有分叉时）输出：
# Updating 1a2b3c4..7d8e9f0
# Fast-forward
#  src/login.cpp | 30 ++++++++++++++++++++++++++++++
#  1 file changed, 30 insertions(+)

# 三方合并（有分叉时）输出：
# Merge made by the 'ort' strategy.
#  src/login.cpp | 30 ++++++++++++++++++++++++++++++
#  1 file changed, 30 insertions(+)

# 不使用快进合并（保留分支历史）
git merge --no-ff feature-login
# 输出：
# Merge made by the 'ort' strategy.
#  src/login.cpp | 30 ++++++++++++++++++++++++++++++
#  1 file changed, 30 insertions(+)
# （会创建一个合并提交，历史中能看到分支）

# 合并时遇到冲突
git merge feature-api
# 输出：
# Auto-merging src/main.cpp
# CONFLICT (content): Merge conflict in src/main.cpp
# Automatic merge failed; fix conflicts and then commit the result.

# 查看冲突文件
git status
# 输出：
# Unmerged paths:
#   (use "git add <file>..." to mark resolution)
#         both modified:   src/main.cpp

# 冲突文件内容：
# <<<<<<< HEAD
# int main() {
#     login();
# =======
# int main() {
#     init_api();
# >>>>>>> feature-api
# }

# 解决冲突后
git add src/main.cpp
git commit
# 输出：
# [main 9a0b1c2] Merge branch 'feature-api'

# 取消合并
git merge --abort
```

**变基（rebase）：**

```bash
# 在feature分支上变基到main
git checkout feature-login
git rebase main
# 输出：
# Rebasing (1/3)
# Rebasing (2/3)
# Rebasing (3/3)
# Successfully rebased and updated refs/heads/feature-login.

# 变基过程中遇到冲突
git rebase main
# 输出：
# Rebasing (1/3)
# CONFLICT (content): Merge conflict in src/main.cpp
# error: could not apply 3a4b5c6... feat: 添加登录功能
# Resolve all conflicts manually, mark them as resolved with
# "git add/rm <conflicted_files>" then run "git rebase --continue".

# 解决冲突后继续
git add src/main.cpp
git rebase --continue
# 输出：
# Rebasing (2/3)
# Rebasing (3/3)
# Successfully rebased and updated refs/heads/feature-login.

# 跳过当前提交
git rebase --skip

# 放弃变基
git rebase --abort

# 交互式变基（修改历史提交）
git rebase -i HEAD~3
# 编辑器中显示：
# pick 1a2b3c4 feat: 添加登录功能
# pick 3a4b5c6 fix: 修复登录Bug
# pick 7d8e9f0 docs: 更新文档
#
# 可用命令：
# pick   → 使用提交
# reword → 使用提交但修改信息
# edit   → 使用提交但暂停修改
# squash → 合并到前一个提交
# drop   → 丢弃提交
```

### 7. git stash — 临时保存

```bash
# 保存当前工作区的修改
git stash
# 输出：
# Saved working directory and index state WIP on main: 3a4b5c6 feat: 添加登录功能

# 带消息保存
git stash push -m "正在开发注册功能，临时切换分支"
# 输出：
# Saved working directory and index state On main: 正在开发注册功能，临时切换分支

# 包含未跟踪的文件
git stash -u

# 查看所有stash
git stash list
# 输出：
# stash@{0}: On main: 正在开发注册功能，临时切换分支
# stash@{1}: WIP on main: 3a4b5c6 feat: 添加登录功能

# 恢复最近的stash
git stash pop
# 输出：
# On branch main
# Changes not staged for commit:
#   modified:   src/main.cpp
# Dropped refs/stash@{0} (1a2b3c4...)

# 恢复指定stash但不删除
git stash apply stash@{1}
# 输出：
# On branch main
# Changes not staged for commit:
#   modified:   src/main.cpp

# 删除stash
git stash drop stash@{1}
# 输出：
# Dropped stash@{1} (3a4b5c6...)

# 清空所有stash
git stash clear

# 查看stash内容
git stash show -p stash@{0}
# 输出：显示该stash的完整diff
```

### 8. git tag — 打标签

```bash
# 创建轻量标签
git tag v1.0.0
# 无输出表示成功

# 创建附注标签（推荐，包含更多信息）
git tag -a v1.0.0 -m "版本1.0.0：首个正式发布版本"
# 无输出表示成功

# 给历史提交打标签
git tag -a v0.9.0 1a2b3c4 -m "版本0.9.0：测试版本"

# 查看所有标签
git tag
# 输出：
# v0.9.0
# v1.0.0
# v1.1.0
# v2.0.0

# 按模式筛选标签
git tag -l "v1.*"
# 输出：
# v1.0.0
# v1.1.0

# 查看标签信息
git show v1.0.0
# 输出：
# tag v1.0.0
# Tagger: 张三 <zhangsan@example.com>
# Date:   Thu May 21 14:00:00 2026 +0800
#
# 版本1.0.0：首个正式发布版本
#
# commit 3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b (tag: v1.0.0)
# Author: 张三 <zhangsan@example.com>
# ...

# 推送标签到远程
git push origin v1.0.0
# 输出：
# To https://github.com/user/repo.git
#  * [new tag]         v1.0.0 -> v1.0.0

# 推送所有标签
git push origin --tags

# 删除本地标签
git tag -d v0.9.0
# 输出：
# Deleted tag 'v0.9.0' (was 1a2b3c4)

# 删除远程标签
git push origin --delete v0.9.0
# 输出：
# To https://github.com/user/repo.git
#  - [deleted]         v0.9.0

# 检出标签（进入分离头指针状态）
git checkout v1.0.0
# 输出：
# Note: switching to 'v1.0.0'.
# You are in 'detached HEAD' state.
```

语义化版本号：

```text
v主版本号.次版本号.修订号

v2.1.3
│ │ │
│ │ └── 修订号：Bug修复（向后兼容）
│ └──── 次版本号：新功能（向后兼容）
└────── 主版本号：重大变更（可能不兼容）

附加标识：
  v1.0.0-alpha   → 内部测试版
  v1.0.0-beta    → 公开测试版
  v1.0.0-rc.1    → 候选发布版
```

### 9. git reset / revert — 撤销操作

**reset（重置到指定提交）：**

```bash
# 查看提交历史，找到要回退的版本
git log --oneline
# 输出：
# 7d8e9f0 feat: 添加注册功能
# 3a4b5c6 feat: 添加登录功能
# 1a2b3c4 init: 初始化项目

# 软重置：保留修改在暂存区（只移动HEAD指针）
git reset --soft 1a2b3c4
# 输出：无
# 效果：HEAD → 1a2b3c4，3a4b5c6和7d8e9f0的修改在暂存区
git status
# 输出：
# Changes to be committed:
#   new file:   src/login.cpp
#   new file:   src/register.cpp

# 混合重置（默认）：保留修改在工作区（不暂存）
git reset --mixed 1a2b3c4
# 或简写为：
git reset 1a2b3c4
# 效果：HEAD → 1a2b3c4，修改在工作区但未暂存
git status
# 输出：
# Untracked files:
#   src/login.cpp
#   src/register.cpp

# 硬重置：丢弃所有修改（危险！）
git reset --hard 1a2b3c4
# 效果：HEAD → 1a2b3c4，所有修改全部丢失
git status
# 输出：
# On branch main
# nothing to commit, working tree clean

# 撤销暂存（把文件从暂存区移回工作区）
git reset HEAD src/main.cpp
# 输出：
# Unstaged changes after reset:
# M      src/main.cpp

# 回退一个提交
git reset HEAD~1

# 回退两个提交
git reset HEAD~2
```

三种reset对比：

```text
                    工作区    暂存区    仓库
--soft              保留      保留      回退
--mixed（默认）      保留      回退      回退
--hard              回退      回退      回退
```

**revert（创建新提交来撤销）：**

```bash
# 撤销指定提交（创建一个新的撤销提交）
git revert 3a4b5c6
# 输出：
# [main 9a0b1c2] Revert "feat: 添加登录功能"
#  1 file changed, 0 insertions(+), 30 deletions(-)
#  delete mode 100644 src/login.cpp

# 撤销多个提交
git revert 3a4b5c6 7d8e9f0

# 撤销但不自动提交（可以合并多个revert）
git revert --no-commit 3a4b5c6
git revert --no-commit 7d8e9f0
git commit -m "revert: 撤销登录和注册功能"

# 撤销最近一次提交
git revert HEAD
```

reset vs revert：

```text
reset：改写历史，适合未推送的本地提交
revert：创建新提交，适合已推送的公共提交

已推送到远程？ → 用 revert（安全，不改写历史）
仅本地提交？   → 用 reset（干净，改写历史）
```

### 10. git cherry-pick — 精选提交

```bash
# 查看其他分支的提交
git log --oneline feature-login
# 输出：
# 7d8e9f0 fix: 修复密码验证逻辑
# 3a4b5c6 feat: 添加密码加密
# 1a2b3c4 feat: 添加登录页面

# 只挑选某个提交到当前分支
git cherry-pick 3a4b5c6
# 输出：
# [main a1b2c3d] feat: 添加密码加密
#  Date: Thu May 21 10:00:00 2026 +0800
#  1 file changed, 20 insertions(+)

# 挑选多个提交
git cherry-pick 1a2b3c4 3a4b5c6
# 输出：
# [main d4e5f6a] feat: 添加登录页面
#  1 file changed, 30 insertions(+)
# [main a1b2c3d] feat: 添加密码加密
#  1 file changed, 20 insertions(+)

# 挑选提交范围
git cherry-pick 1a2b3c4..3a4b5c6

# cherry-pick遇到冲突
git cherry-pick 7d8e9f0
# 输出：
# error: could not apply 7d8e9f0... fix: 修复密码验证逻辑
# hint: Resolve all conflicts manually...
# CONFLICT (content): Merge conflict in src/auth.cpp

# 解决冲突后继续
git add src/auth.cpp
git cherry-pick --continue

# 放弃cherry-pick
git cherry-pick --abort
```

## 5. GitHub操作

### 1. 创建仓库

```bash
# 方式一：在GitHub网页上创建
# 1. 登录GitHub → 右上角 "+" → "New repository"
# 2. 填写仓库名、描述
# 3. 选择公开/私有
# 4. 可选：添加README、.gitignore、License
# 5. 点击 "Create repository"

# 方式二：使用GitHub CLI
gh repo create my-project --public --description "我的项目"
# 输出：
# ✓ Created repository zhangsan/my-project on GitHub
# https://github.com/zhangsan/my-project

# 创建后关联本地仓库
git remote add origin https://github.com/zhangsan/my-project.git
git push -u origin main

# 查看远程仓库
git remote -v
# 输出：
# origin  https://github.com/zhangsan/my-project.git (fetch)
# origin  https://github.com/zhangsan/my-project.git (push)

# 修改远程仓库地址
git remote set-url origin git@github.com:zhangsan/my-project.git
```

### 2. Fork和Pull Request流程

```text
完整Fork → PR流程：

1. Fork别人的仓库
   GitHub页面 → Fork按钮 → 创建到自己账号下

2. 克隆自己Fork的仓库
   git clone https://github.com/你的用户名/项目名.git

3. 添加上游仓库
   git remote add upstream https://github.com/原作者/项目名.git

4. 创建功能分支
   git checkout -b fix-bug-123

5. 修改代码并提交
   git add .
   git commit -m "fix: 修复Bug #123"

6. 推送到自己的Fork
   git push origin fix-bug-123

7. 在GitHub上创建Pull Request
   GitHub页面 → New Pull Request → 填写描述 → 提交

8. 同步上游更新
   git fetch upstream
   git checkout main
   git merge upstream/main
```

```bash
# 使用GitHub CLI创建PR
gh pr create --title "fix: 修复内存泄漏" --body "修复了utils.cpp中的内存泄漏问题"
# 输出：
# Creating pull request for fix-memory-leak into main in zhangsan/my-project
#
# https://github.com/zhangsan/my-project/pull/42

# 查看PR列表
gh pr list
# 输出：
# Showing 3 of 3 open pull requests in zhangsan/my-project
#
# #42  fix: 修复内存泄漏        fix-memory-leak  about 2 hours ago
# #41  feat: 添加日志模块        feature-logging  about 1 day ago
# #40  docs: 更新API文档         docs-api         about 3 days ago

# 检出PR到本地
gh pr checkout 42
# 输出：
# Switched to branch 'fix-memory-leak'
```

### 3. Issues管理

```bash
# 创建Issue
gh issue create --title "Bug: 登录页面崩溃" --body "在Chrome浏览器中点击登录按钮后页面崩溃"
# 输出：
# https://github.com/zhangsan/my-project/issues/15

# 查看Issue列表
gh issue list
# 输出：
# Showing 3 of 3 open issues in zhangsan/my-project
#
# #15  Bug: 登录页面崩溃    about 1 minute ago
# #14  优化数据库查询性能     about 2 hours ago
# #13  添加导出CSV功能       about 1 day ago

# 关闭Issue（在提交信息中引用）
git commit -m "fix: 修复登录崩溃问题，关闭 #15"
# 推送后Issue #15会自动关闭

# 查看Issue详情
gh issue view 15
```

Issue标签管理：

```text
常见标签：
  bug         → Bug报告（红色）
  feature     → 新功能请求（蓝色）
  enhancement → 改进建议（浅蓝色）
  docs        → 文档相关（灰色）
  good first issue → 适合新贡献者（绿色）
  help wanted → 需要帮助（橙色）
  wontfix     → 不会修复（灰色）
  duplicate   → 重复Issue（灰色）
```

### 4. GitHub Actions基础

在仓库中创建 `.github/workflows/build.yml`：

```yaml
name: C++构建与测试

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
    - name: 检出代码
      uses: actions/checkout@v4

    - name: 安装依赖
      run: |
        sudo apt update
        sudo apt install -y cmake g++

    - name: 配置CMake
      run: cmake -B build -DCMAKE_BUILD_TYPE=Release

    - name: 编译
      run: cmake --build build --parallel

    - name: 运行测试
      run: cd build && ctest --output-on-failure
```

```bash
# 查看Actions运行状态
gh run list
# 输出：
# STATUS  NAME           BRANCH  EVENT  ID
# ✓       C++构建与测试   main    push   12345678
# ✗       C++构建与测试   develop push   12345677

# 查看某次运行的详情
gh run view 12345678
```

### 5. .gitignore文件

```text
# .gitignore 文件示例

# 编译产物
*.o
*.obj
*.exe
*.out
*.app
*.so
*.dylib
*.dll

# 构建目录
build/
cmake-build-*/
out/

# IDE配置
.vscode/
.idea/
*.swp
*.swo
*~

# 操作系统文件
.DS_Store
Thumbs.db
desktop.ini

# 依赖目录
node_modules/
vendor/

# 环境配置（包含敏感信息）
.env
.env.local

# 日志文件
*.log

# 调试文件
*.dSYM/
*.su
*.idb
*.pdb

# 但保留某个文件（用!取反）
!important-config.o
```

```bash
# .gitignore规则不生效时的处理
# 原因：文件已经被Git跟踪，.gitignore只对未跟踪的文件生效

# 取消跟踪（保留本地文件）
git rm --cached src/config.h
git commit -m "chore: 从版本控制中移除配置文件"

# 取消跟踪整个目录
git rm -r --cached build/
git commit -m "chore: 从版本控制中移除构建目录"
```

### 6. README.md写法

```markdown
# 项目名称

简短的项目描述。

## 6. 功能特性

- 特性一：支持XXX
- 特性二：支持YYY

## 7. 快速开始

### 26. 环境要求

- C++17或更高版本
- CMake 3.20+
- GCC 11+ / Clang 14+ / MSVC 2022+

### 27. 编译安装

```bash
git clone https://github.com/user/project.git
cd project
cmake -B build
cmake --build build --parallel
```

### 28. 运行

```bash
./build/project
```

## 8. 使用示例

```cpp
#include "project/api.h"

int main() {
    Project::init();
    Project::run();
    return 0;
}
```

## 9. 项目结构

```
project/
├── src/          # 源代码
├── include/      # 头文件
├── tests/        # 测试代码
├── docs/         # 文档
└── CMakeLists.txt
```

## 10. 贡献指南

1. Fork本仓库
2. 创建功能分支 (`git checkout -b feature/amazing-feature`)
3. 提交修改 (`git commit -m 'feat: 添加某某功能'`)
4. 推送分支 (`git push origin feature/amazing-feature`)
5. 创建Pull Request

## 11. 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件
```

## 6. 团队协作流程

### 1. Git Flow工作流

```text
Git Flow：适合有计划发布周期的项目

  main        ──────────────────────────────────  ← 生产代码
                ↑           ↑           ↑
  release/1.0 ──┘           │           │        ← 发布准备
                          merge       merge
  develop     ──────────────────────────────────  ← 开发主线
                ↑         ↑
  feature/A  ──┘         │                       ← 功能分支
  feature/B            ──┘                       ← 功能分支

  hotfix/xxx  ──────────────────┐                ← 紧急修复
                               │
  main        ─────────────────┘

分支类型：
  main        → 生产环境代码，只接受merge
  develop     → 开发主线，集成所有功能
  feature/*   → 功能分支，从develop创建
  release/*   → 发布分支，从develop创建
  hotfix/*    → 热修复，从main创建
```

```bash
# Git Flow操作示例

# 创建功能分支
git checkout develop
git checkout -b feature/user-auth

# 开发完成后合并回develop
git checkout develop
git merge --no-ff feature/user-auth
git branch -d feature/user-auth

# 创建发布分支
git checkout -b release/1.0.0 develop
# 修复Bug、更新版本号...

# 发布分支合并到main和develop
git checkout main
git merge --no-ff release/1.0.0
git tag -a v1.0.0 -m "版本1.0.0"

git checkout develop
git merge --no-ff release/1.0.0
git branch -d release/1.0.0

# 紧急修复
git checkout -b hotfix/critical-bug main
# 修复Bug...
git checkout main
git merge --no-ff hotfix/critical-bug
git tag -a v1.0.1 -m "版本1.0.1：紧急修复"

git checkout develop
git merge --no-ff hotfix/critical-bug
git branch -d hotfix/critical-bug
```

### 2. GitHub Flow工作流

```text
GitHub Flow：更简单，适合持续部署的项目

  main  ────────────────────────────────────  ← 随时可部署
          ↑         ↑         ↑
  feature/A ───────┘         │
  feature/B ─────────────────┘              ← PR合并后自动部署

规则：
  1. main分支永远可部署
  2. 所有开发在功能分支上进行
  3. 通过Pull Request进行代码审查
  4. 合并后立即部署
```

```bash
# GitHub Flow操作示例

# 创建功能分支
git checkout -b feature/add-search main

# 开发、提交、推送
git add .
git commit -m "feat: 添加搜索功能"
git push -u origin feature/add-search

# 在GitHub上创建Pull Request
gh pr create --title "feat: 添加搜索功能" --body "实现了全文搜索功能"

# 代码审查通过后合并（GitHub网页操作或CLI）
gh pr merge 42 --merge
# 输出：
# ✓ Merged pull request #42 (feat: 添加搜索功能)

# 删除远程分支
git push origin --delete feature/add-search

# 同步本地
git checkout main
git pull
git branch -d feature/add-search
```

### 3. 代码审查流程

```text
代码审查（Code Review）流程：

1. 开发者创建PR
   ↓
2. 自动化检查（CI/CD、代码风格检查）
   ↓
3. 至少一位审查者审查代码
   ↓
4. 审查者提出修改意见（Comment / Request Changes）
   ↓
5. 开发者根据意见修改并推送新提交
   ↓
6. 审查者确认后Approve
   ↓
7. 合并PR
```

```bash
# 审查者操作

# 检出PR到本地
gh pr checkout 42

# 查看变更
git diff main...feature/add-search

# 在终端中审查
gh pr view 42 --comments

# 提交审查意见
gh pr review 42 --comment -b "代码整体不错，有几处建议：\n1. 建议使用const引用传递大对象\n2. 错误处理可以更完善"

# 请求修改
gh pr review 42 --request-changes -b "请修复内存泄漏问题"

# 批准合并
gh pr review 42 --approve -b "代码质量很好，批准合并"
```

审查清单：

```text
代码审查关注点：
  □ 代码逻辑是否正确
  □ 是否有内存泄漏或资源未释放
  □ 错误处理是否完善
  □ 代码风格是否一致
  □ 是否有安全隐患
  □ 性能是否有问题
  □ 是否有足够的测试
  □ 命名是否清晰
  □ 是否有冗余代码
  □ 文档是否需要更新
```

### 4. 解决冲突

```bash
# 场景：合并时出现冲突

git merge feature-login
# 输出：
# Auto-merging src/main.cpp
# CONFLICT (content): Merge conflict in src/main.cpp
# Automatic merge failed; fix conflicts and then commit the result.

# 第一步：查看冲突文件
git status
# 输出：
# Unmerged paths:
#   both modified:   src/main.cpp

# 第二步：打开冲突文件
# 冲突标记格式：
# <<<<<<< HEAD           ← 当前分支的内容
# 当前分支的代码
# =======                ← 分隔线
# 合并分支的代码
# >>>>>>> feature-login  ← 合并分支的内容

# 第三步：手动解决冲突
# 保留需要的代码，删除冲突标记

# 冲突前：
# <<<<<<< HEAD
# #include "login.h"
# #include "database.h"
# =======
# #include "login.h"
# #include "cache.h"
# >>>>>>> feature-login

# 解决后（保留两边的修改）：
# #include "login.h"
# #include "database.h"
# #include "cache.h"

# 第四步：标记冲突已解决
git add src/main.cpp

# 第五步：完成合并
git commit
# 输出：
# [main 9a0b1c2] Merge branch 'feature-login'

# 使用VS Code解决冲突（推荐）
# VS Code会提供 "Accept Current" / "Accept Incoming" / "Accept Both" 按钮

# 使用mergetool
git mergetool
# 输出：打开配置的合并工具

# 完全使用某一方的版本
git checkout --ours src/main.cpp      # 使用当前分支的版本
git checkout --theirs src/main.cpp    # 使用合并分支的版本
```

## 7. 常见问题

### 1. push被拒绝

```bash
# 错误场景
git push
# 输出：
# To https://github.com/user/repo.git
#  ! [rejected]        main -> main (fetch first)
# error: failed to push some refs to 'https://github.com/user/repo.git'
# hint: Updates were rejected because the remote contains work that you do
# hint: not have locally.

# 原因：远程仓库有本地没有的新提交

# 解决方式一：先拉取再推送
git pull --rebase origin main
# 输出：
# remote: Enumerating objects: 5, done.
# ...
# Rebasing (1/2)
# Rebasing (2/2)
# Successfully rebased and updated refs/heads/main.

git push
# 输出：
# To https://github.com/user/repo.git
#    1a2b3c4..7d8e9f0  main -> main

# 解决方式二：强制推送（仅限个人分支，危险！）
git push -f origin feature-login

# 解决方式三：先fetch查看差异再决定
git fetch origin
git log HEAD..origin/main
git merge origin/main
git push
```

### 2. 合并冲突怎么解决

```bash
# 最常见的冲突场景和解决方案

# 场景一：同一文件同一位置被不同人修改
# 解决：手动选择保留哪部分代码

# 场景二：一方修改了文件，另一方删除了文件
git merge feature-branch
# 输出：
# CONFLICT (delete/modify): src/old-module.cpp deleted in feature-branch and modified in HEAD

# 保留文件
git add src/old-module.cpp

# 删除文件
git rm src/old-module.cpp

# 场景三：二进制文件冲突（图片、编译产物等）
# 解决：选择保留其中一个版本
git checkout --ours images/logo.png
git add images/logo.png

# 预防冲突的最佳实践：
# 1. 频繁拉取远程更新
# 2. 小步提交，减少每次修改范围
# 3. 团队约定文件分工，避免多人改同一文件
# 4. 使用功能分支而非直接在main上开发
```

### 3. 误提交怎么撤销

```bash
# 场景一：提交信息写错了
git commit --amend -m "fix: 正确的提交信息"
# 输出：
# [main 7d8e9f0] fix: 正确的提交信息
#  Date: Thu May 21 10:00:00 2026 +0800
#  1 file changed, 5 insertions(+)

# 场景二：漏提交了文件
git add forgotten-file.cpp
git commit --amend --no-edit
# 输出：
# [main 7d8e9f0] fix: 修复Bug
#  Date: Thu May 21 10:00:00 2026 +0800
#  2 files changed, 10 insertions(+)

# 场景三：撤销最近一次提交（保留修改）
git reset --soft HEAD~1

# 场景四：撤销最近一次提交（丢弃修改）
git reset --hard HEAD~1

# 场景五：撤销已推送的提交（公共分支，安全方式）
git revert HEAD
# 输出：
# [main 9a0b1c2] Revert "fix: 修复Bug"
#  1 file changed, 0 insertions(+), 5 deletions(-)

# 场景六：撤销某个历史提交
git revert 3a4b5c6

# 场景七：误操作了git reset --hard，想恢复
# 使用reflog找回
git reflog
# 输出：
# 7d8e9f0 HEAD@{0}: reset: moving to HEAD~1
# 3a4b5c6 HEAD@{1}: commit: feat: 添加登录功能  ← 这就是丢失的提交
# 1a2b3c4 HEAD@{2}: commit: init: 初始化项目

# 恢复到reset之前的状态
git reset --hard 3a4b5c6
# 输出：
# HEAD is now at 3a4b5c6 feat: 添加登录功能
```

### 4. 大文件处理（Git LFS）

```bash
# 问题：Git不适合管理大文件（模型、视频、数据集等）
# GitHub单文件限制100MB，仓库总大小建议不超过1GB

# 安装Git LFS
git lfs install
# 输出：
# Git LFS initialized.

# 跟踪大文件类型
git lfs track "*.psd"
# 输出：
# Tracking "*.psd"

git lfs track "*.model"
git lfs track "datasets/**"

# 查看跟踪规则
cat .gitattributes
# 输出：
# *.psd filter=lfs diff=lfs merge=lfs -text
# *.model filter=lfs diff=lfs merge=lfs -text
# datasets/** filter=lfs diff=lfs merge=lfs -text

# 提交.gitattributes
git add .gitattributes
git commit -m "chore: 配置Git LFS跟踪规则"

# 正常添加大文件
git add large-model.model
git commit -m "feat: 添加AI模型文件"
git push

# 查看LFS文件列表
git lfs ls-files
# 输出：
# 3a4b5c6d - large-model.model

# 查看LFS存储使用量
git lfs env
```

### 5. 敏感信息误提交

```bash
# 场景：不小心把密码、密钥提交到了仓库

# 方法一：git filter-branch（Git内置，速度较慢）
# 从所有历史中删除文件
git filter-branch --force --index-filter \
  'git rm --cached --ignore-unmatch src/config-with-secrets.h' \
  --prune-empty -- --all
# 输出：
# Rewrite 3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b (3/3)
# Ref 'refs/heads/main' was rewritten

# 从所有历史中替换文件中的敏感字符串
git filter-branch --tree-filter '
  if [ -f src/config.h ]; then
    sed -i "s/MY_SECRET_PASSWORD/REDACTED/g" src/config.h
  fi
' --force -- --all

# 方法二：BFG Repo-Cleaner（更快，推荐）
# 安装：下载 bfg.jar

# 删除包含敏感信息的文件
java -jar bfg.jar --delete-files config-with-secrets.h
# 输出：
# Cleaning
# --------
# Found 2 commits to clean
# ...
# BFG run is complete

# 替换敏感文本
java -jar bfg.jar --replace-text replacements.txt
# replacements.txt 内容：
# MY_SECRET_PASSWORD==>REDACTED
# API_KEY_12345==>REDACTED

# 清理和垃圾回收
git reflog expire --expire=now --all
git gc --prune=now --aggressive
# 输出：
# Enumerating objects: 30, done.
# Counting objects: 100% (30/30), done.
# Delta compression using up to 8 threads
# Compressing objects: 100% (20/20), done.
# Writing objects: 100% (30/30), done.

# 强制推送覆盖远程
git push --force --all

# 重要提醒：
# 1. 清理历史后，所有协作者需要重新克隆仓库
# 2. 已泄露的密钥必须立即更换，仅删除历史不够
# 3. 操作前先备份仓库
```

预防敏感信息提交：

```bash
# 使用全局的.gitignore排除敏感文件
git config --global core.excludesFile ~/.gitignore_global

# 在 ~/.gitignore_global 中添加：
# .env
# *.key
# *.pem
# *.secret
# credentials.json

# 使用git-secrets工具（AWS开源）
git secrets --install
git secrets --register-aws

# 使用pre-commit钩子检查
# 创建 .git/hooks/pre-commit
```

```bash
#!/bin/bash
# .git/hooks/pre-commit
# 检查暂存区是否包含敏感信息

if git diff --cached | grep -iE '(password|secret|api_key|token)\s*=\s*["\x27]'; then
    echo "错误：检测到可能的敏感信息，请检查代码！"
    exit 1
fi
```

### 6. 其他实用技巧

```bash
# 查看某行代码是谁写的
git blame src/main.cpp
# 输出：
# 3a4b5c6d (张三 2026-05-20 10:00:00 +0800 1) #include <iostream>
# 7d8e9f0a (李四 2026-05-21 14:30:00 +0800 2) #include <vector>
# 3a4b5c6d (张三 2026-05-20 10:00:00 +0800 3) int main() {

# 查看某行范围的blame
git blame -L 10,20 src/main.cpp

# 查看某个文件的完整修改历史（每次修改的差异）
git log -p --follow src/main.cpp

# 搜索代码内容在历史中的变更
git log -S "password" --source --all

# 查看某次提交的详细内容
git show 3a4b5c6
# 输出：
# commit 3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b
# Author: 张三 <zhangsan@example.com>
# Date:   Thu May 21 10:00:00 2026 +0800
#
#     feat: 添加登录功能
#
# diff --git a/src/main.cpp b/src/main.cpp
# ...

# 临时忽略文件权限变更
git config core.fileMode false

# 清理未跟踪的文件（预览）
git clean -n
# 输出：
# Would remove untracked-file.txt
# Would remove build/

# 清理未跟踪的文件（执行）
git clean -fd
# 输出：
# Removing untracked-file.txt
# Removing build/

# 清理忽略的文件
git clean -fdX

# 查看仓库大小
git count-objects -vH
# 输出：
# count: 150
# size: 2.50 MiB
# in-pack: 300
# pack-size: 1.80 MiB
# size-pack: 1.80 MiB
# prune-packable: 0
# garbage: 0
# size-garbage: 0 bytes

# 二分查找引入Bug的提交
git bisect start
git bisect bad                  # 当前版本有Bug
git bisect good v1.0.0          # v1.0.0版本没有Bug
# Git会自动检出中间的提交
# 测试后标记：
git bisect good                 # 这个版本正常
# 或
git bisect bad                  # 这个版本有Bug
# 重复直到找到引入Bug的提交
# 输出：
# 3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9e0f1a2b is the first bad commit

# 结束二分查找
git bisect reset

# 自动化二分查找
git bisect start HEAD v1.0.0
git bisect run ./test-script.sh
```

### 7. 常用命令速查表

```text
基础操作：
  git init                    初始化仓库
  git clone <url>             克隆远程仓库
  git add <file>              暂存文件
  git commit -m "msg"         提交
  git status                  查看状态
  git log                     查看日志
  git diff                    查看差异

分支操作：
  git branch                  列出分支
  git branch <name>           创建分支
  git checkout <branch>       切换分支
  git switch <branch>         切换分支（新语法）
  git merge <branch>          合并分支
  git rebase <branch>         变基
  git branch -d <name>        删除分支

远程操作：
  git remote -v               查看远程仓库
  git fetch                   拉取远程更新
  git pull                    拉取并合并
  git push                    推送到远程
  git push -u origin main     首次推送并设置上游

撤销操作：
  git reset --soft HEAD~1     软重置（保留暂存）
  git reset --mixed HEAD~1    混合重置（保留工作区）
  git reset --hard HEAD~1     硬重置（全部丢弃）
  git revert <commit>         创建撤销提交
  git checkout -- <file>      丢弃工作区修改
  git restore <file>          丢弃工作区修改（新语法）
  git restore --staged <file> 取消暂存（新语法）

暂存操作：
  git stash                   保存当前修改
  git stash list              查看暂存列表
  git stash pop               恢复并删除
  git stash apply             恢复但不删除

标签操作：
  git tag                     列出标签
  git tag -a v1.0 -m "msg"   创建附注标签
  git push origin v1.0        推送标签
  git tag -d v1.0             删除本地标签

高级操作：
  git cherry-pick <commit>    精选提交
  git rebase -i HEAD~3        交互式变基
  git bisect                  二分查找Bug
  git blame <file>            查看行级作者
  git reflog                  查看操作日志
  git stash                   临时保存
```

***

### 8. 相关章节

- [代码审查](../04-工程实践/08-代码审查.md) — Git PR中的代码审查实践
- [编码规范](../04-工程实践/00-编码规范.md) — 配合Git hooks的编码规范检查
- [VS Code开发环境完全配置指南](../05-开发环境与IDE/00-VSCode核心配置.md) — VS Code内置Git功能
- [CMake与构建系统实战配置](../05-开发环境与IDE/03-CMake基础入门.md) — CMake项目与Git配合
- [开发环境配置详解](../01-基础概念/28-开发环境配置.md) — 环境搭建包含Git安装
- [开源恩怨录](../06-编程故事与警示/08-开源恩怨录.md) — 自由软件世界的分裂与争斗

***

### 相关阅读

- [CI-CD与DevOps实践](./23-CI-CD与DevOps实践.md)
- [开源许可协议](./24-开源许可协议.md)
- [CPP编码规范与命名约定](./04-CPP编码规范与命名约定.md)