#!/bin/bash

# 定义要创建的目录结构
DIRS=(
    "css"
    "js"
    "images"
    "assets/fonts"
    "assets/icons"
    "pages/about"
    "pages/contact"
    "posts/2023"
)

# 定义要创建的基本HTML文件
FILES=(
    "index.html"
    "about.html"
    "contact.html"
    "404.html"
    "css/style.css"
    "js/main.js"
    "posts/2023/post1.html"
    "posts/2023/post2.html"
)

# 创建目录
echo "创建目录结构..."
for dir in "${DIRS[@]}"; do
    mkdir -p "$dir"
    echo "  + $dir/"
done

# 创建文件并写入基本HTML结构
echo "生成HTML文件..."
for file in "${FILES[@]}"; do
    cat <<EOF > "$file"
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>${file%.*}</title>
    <link rel="stylesheet" href="/css/style.css">
</head>
<body>
    <h1>${file%.*} Page</h1>
    <p>This is a generated test file: $file</p>
    <script src="/js/main.js"></script>
</body>
</html>
EOF
    echo "  + $file"
done

# 创建示例图片
echo "生成占位图片..."
convert -size 100x100 xc:gray images/placeholder1.jpg 2>/dev/null ||
    echo "  ! 需要安装ImageMagick生成图片，跳过"
convert -size 200x150 xc:blue images/placeholder2.png 2>/dev/null ||
    echo "  ! 需要安装ImageMagick生成图片，跳过"

echo "完成！生成的文件结构："
tree -L 3