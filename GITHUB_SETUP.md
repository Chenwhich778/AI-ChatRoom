# 🚀 GitHub 仓库创建指南

## 方法一：创建全新的个人仓库（推荐）

### 步骤 1：在 GitHub 网站创建新仓库

1. 访问 [GitHub](https://github.com)
2. 点击右上角的 `+` 号，选择 `New repository`
3. 填写仓库信息：
   - **Repository name**: `DuckChat`（或你喜欢的名字）
   - **Description**: `一个功能完整的多人聊天室应用，基于 Qt 6 开发，集成 AI 助手`
   - **Public/Private**: 选择 Public（公开）或 Private（私有）
   - **⚠️ 重要**: 不要勾选 "Initialize this repository with a README"
   - 不要添加 .gitignore 和 license（我们已经有了）
4. 点击 `Create repository`

### 步骤 2：断开原仓库连接

由于当前项目已连接到原作者的仓库，需要先断开：

```bash
cd E:\Workspace\qt-project\DuckChat

# 查看当前远程仓库
git remote -v

# 删除原来的 origin
git remote remove origin
```

### 步骤 3：连接到你的新仓库

将 `your-username` 替换为你的 GitHub 用户名：

```bash
# 添加你的新仓库为远程仓库
git remote add origin https://github.com/your-username/DuckChat.git

# 验证连接
git remote -v
```

### 步骤 4：提交并推送代码

```bash
# 添加 .gitignore 和 API 配置指南
git add .gitignore API_CONFIG_GUIDE.md

# 添加修改的源代码文件
git add Client/mainwindow.cpp Client/mainwindow.h Client/aiassistant.cpp Client/aiassistant.h
git add Server/server.cpp Server/server.h

# 添加更新的 README
git add README.md

# 提交更改
git commit -m "feat: 添加多聊天室支持和 AI 助手功能

- 实现用户可同时加入多个聊天室
- 集成 SiliconFlow AI API
- 添加 API 密钥安全配置方案
- 更新 README 和文档
- 修复多个 bug"

# 推送到 GitHub（首次推送）
git branch -M main
git push -u origin main
```

### 步骤 5：验证

访问 `https://github.com/your-username/DuckChat` 查看你的新仓库！

---

## 方法二：保持fork关系（如果你想保留原项目的贡献记录）

如果你想保留对原项目的引用和 fork 关系：

### 步骤 1：在 GitHub 上 Fork

1. 访问原项目页面
2. 点击右上角 `Fork` 按钮
3. 选择你的账号

### 步骤 2：更新本地仓库连接

```bash
cd E:\Workspace\qt-project\DuckChat

# 删除原来的 origin
git remote remove origin

# 添加你 fork 的仓库
git remote add origin https://github.com/your-username/DuckChat.git

# 添加原作者仓库为 upstream（可选）
git remote add upstream https://github.com/original-author/DuckChat.git
```

### 步骤 3：推送代码

```bash
# 添加所有更改
git add .gitignore API_CONFIG_GUIDE.md
git add Client/ Server/
git add README.md

# 提交
git commit -m "feat: 重大更新 - 多聊天室和AI功能"

# 推送
git push -u origin master
```

---

## 🔒 安全检查清单

在推送前，确保：

- ✅ `.gitignore` 文件包含 `**/config/`
- ✅ API 密钥文件 `config/api.conf` 不在 Git 跟踪中
- ✅ `build/` 目录被忽略
- ✅ 没有硬编码的密钥或敏感信息

检查命令：

```bash
# 查看将要提交的文件
git status

# 确保 config/ 目录不在列表中
git ls-files | grep config

# 应该没有输出，如果有输出说明 config 文件被跟踪了
```

---

## 📝 后续维护

### 每次代码更新后：

```bash
# 1. 查看更改
git status

# 2. 添加更改的文件
git add <文件名>

# 3. 提交
git commit -m "描述你的更改"

# 4. 推送
git push
```

### 常用 commit 消息格式：

- `feat: 添加新功能` - 新功能
- `fix: 修复bug` - Bug修复
- `docs: 更新文档` - 文档更新
- `style: 代码格式调整` - 格式调整
- `refactor: 代码重构` - 重构
- `perf: 性能优化` - 性能优化
- `test: 测试相关` - 测试

---

## 🆘 常见问题

### Q: 如何删除 Git 历史中的敏感文件？

如果不小心提交了 API 密钥：

```bash
# 从历史中删除文件
git filter-branch --force --index-filter \
  "git rm --cached --ignore-unmatch config/api.conf" \
  --prune-empty --tag-name-filter cat -- --all

# 强制推送（⚠️ 危险操作）
git push origin --force --all
```

### Q: 如何完全重新开始？

```bash
# 删除 .git 目录
rm -rf .git

# 重新初始化
git init
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin https://github.com/your-username/DuckChat.git
git push -u origin main --force
```

---

## ✨ 完成！

现在你拥有了一个独立的 GitHub 仓库，可以：

- ✅ 自由修改和发布
- ✅ 接收 Issues 和 Pull Requests
- ✅ 展示在你的 GitHub 个人主页
- ✅ 添加到你的简历和作品集

记得在仓库设置中添加 Topics（如 `qt`, `cpp`, `chat-application`, `ai`, `tcp`）来增加可见度！
