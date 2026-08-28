const extIconMap = {
    //文档类
    xls: 'icon-format-exl',
    xlsx: 'icon-format-xlsx',
    doc: 'icon-format-doc',
    docx: 'icon-format-doc',
    pdf: 'icon-format-pdf',
    txt: 'icon-format-txt',
    //图片类
    bmp: 'icon-format-bmp',
    gif: 'icon-format-gif',
    tif: 'icon-format-tif',
    jpg: 'icon-format-jpeg',
    jpeg: 'icon-format-jpeg',
    png: 'icon-format-png',
    //压缩包
    zip: 'icon-format-zip',
    //追加
    lua: 'icon-lua',
    html: 'icon-html',
    css: 'icon-CSS',
    js: 'icon-JS',
    mp4: 'icon-MP',
    mp3: 'icon-MP1',
    woff2: 'icon-woff',
    webp: 'icon-WEBP',
    ico: 'icon-ICO',
    md: 'icon-MD',
    ttf: 'icon-TTF',
    xml: 'icon-XML',
    yml: 'icon-YML',
    json: 'icon-json-full',
    flac: 'icon-FLAC',
    //默认通用文件
    default: 'icon-wenjian'
};

// 直接用变量判断，不用读地址栏
function isStorageMode() {
    // 兼容数字 1 / 字符串 "1"，不会全局强制
    return window._STORAGE == 1;
}

// 核心工具函数：给 URL 添加 prefix 参数（仅内部存储模式需要）
function addPrefix(url) {
    if (!isStorageMode()) return url;
    if (url.indexOf('prefix=') !== -1) return url;
    if (url.indexOf('?') > -1) {
        return url + '&prefix=/storage';
    } else {
        return url + '?prefix=/storage';
    }
}

let totalFilesCount = 0;
let selectedFilesForDownload = [];
const allowedExtensionsForModify = ['txt', 'html', 'css', 'js', 'json', 'yml', 'scss', 'xml', 'md', 'log', 'csv', 'ini', 'cfg', 'conf', 'sh', 'bat', 'py', 'lua', 'php', 'sql', 'htaccess'];
const allowedExtensionsForModifyPicture = ['jpg', 'jpeg', 'png', 'gif', 'bmp', 'webp', 'svg', 'ico'];
let downloadAbortController = null;

// ==========================
// ZIP 内部浏览（只在这里定义一次！）
// ==========================
let zipInnerDataCache = [];
let zipRootFullPath = "";
let zipPrevRealPath = "";

function storage() {
    zipCache = [];
    zipBasePath = "";
    originalParentPath = "";
    fetch(addPrefix('/get_RootFile'))
        .then(response => response.json())
        .then(data => {
            // ✔️ 正确：直接传 路径 / 进去
            displayFileList(data.files, '/');
        })
        .catch(error => console.error('Error:', error));
}
// 修复 getFiles
function getFiles() {
    // 确保有值，防止 undefined
    if (typeof currentFolderPath === 'undefined' || currentFolderPath === null) {
        currentFolderPath = '/';
    }

    zipCache = []; zipBasePath = ""; originalParentPath = "";

    const sendPath = encodeURIComponent(currentFolderPath);
    const requestUrl = addPrefix('/get_File?path=' + sendPath);

    fetch(requestUrl)
        .then(response => response.json())
        .then(data => {
            // 必须把 currentFolderPath 传进去
            displayFileList(data.files, currentFolderPath);
        })
        .catch(err => console.error(err));
}

// ✅ 删除弹窗（自定义，无系统弹窗）
function confirmDeleteMultipleFiles() {
    const delBtn = document.getElementById('confirmDelete');
    if (delBtn) {
        delBtn.onclick = null;
        delBtn.onclick = function () {
            deleteSelectedFiles();
            document.getElementById('deleteConfirmModal').style.display = 'none';
        };
    }
    document.getElementById('deleteConfirmModal').style.display = 'block';
}

function deleteSelectedFiles() {
    if (selectedFilesForDownload.length === 0) {
        document.getElementById('deleteConfirmModal').style.display = 'none';
        return;
    }

    selectedFilesForDownload.forEach((filePath, index) => {
        deleteSingleFile(filePath, index, selectedFilesForDownload.length);
    });

    showAlert('删除中，请稍等', true);
}

function deleteSingleFile(filePath, currentIndex, totalFiles) {
    const urltocall = addPrefix("/deleteUploadFile?deletePath=" + encodeURIComponent(filePath));
    const xmlhttp = new XMLHttpRequest();

    xmlhttp.onreadystatechange = function () {
        if (this.readyState == 4) {
            if (this.status == 200) {
                if (currentIndex === totalFiles - 1) {
                    selectedFilesForDownload = [];
                    updateFileCountsDisplay();
                    document.getElementById('deleteConfirmModal').style.display = 'none';
                    getFiles();
                    showAlert('已删除', true);
                }
            } else {
                showAlert('删除失败: ' + filePath, false);
            }
        }
    };
    xmlhttp.open("GET", urltocall, true);
    xmlhttp.send();
}

function lookthis(filepath) {
    zipCache = []; zipBasePath = ""; originalParentPath = "";

    const url = addPrefix("/lookthis?lookthisPath=" + encodeURIComponent(filepath));

    fetch(url)
        .then(res => res.json())
        .then(data => {
            // 这里将 data.files 和 你点击的 filepath 传给渲染函数
            displayFileList(data.files, filepath);
            selectedFilesForDownload = [];
        })
        .catch(err => {
            console.error("lookthis 错误", err);
            showAlert("无法进入该目录", false);
        });
}

function backRoot() {
    zipCache = []; zipBasePath = ""; originalParentPath = "";
    window._virtualDirName = null;
    window._virtualZipPath = null;

    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            try {
                var data = JSON.parse(this.responseText);
                // 【核心修改点】：回到根目录时，必须明确传入 '/' 路径参数
                displayFileList(data.files, '/');
                selectedFilesForDownload = [];
            } catch (e) { getFiles(); }
        }
    };
    xmlhttp.open("GET", addPrefix('/backRoot'), true);
    xmlhttp.send();
}

function backone() {
    // 1. 【完美还原】ZIP 内部退出与返回逻辑
    if (zipRootFullPath && currentFolderPath.startsWith(zipRootFullPath)) {
        let parentPath = currentFolderPath.split('/').slice(0, -1).join('/');

        // 如果算出来的上一级等于进入 ZIP 前的真实物理路径，说明要彻底退出 ZIP 虚拟环境
        if (parentPath === zipPrevRealPath) {
            zipRootFullPath = "";
            zipInnerDataCache = [];
            lookthis(zipPrevRealPath || '/'); // 退出 ZIP，回归普通目录
            return;
        }
        // 如果还在 ZIP 深层目录，继续在虚拟 ZIP 里后退
        currentFolderPath = parentPath;
        renderZipCurrentLevel();
        return;
    }

    if (window._virtualDirName) {
        window._virtualDirName = null;
        window._virtualZipPath = null;
        lookthis(zipPrevRealPath || '/');
        return;
    }

    // 2. 【安全计算】普通目录在前端先计算出合法的上一级路径，防止 undefined
    let targetParentPath = '/';
    if (currentFolderPath && currentFolderPath !== '/') {
        targetParentPath = currentFolderPath.substring(0, currentFolderPath.lastIndexOf('/'));
        if (targetParentPath === "") {
            targetParentPath = "/";
        }
    }

    // 3. 【新版适配】带上 currentPath 请求后端
    const url = addPrefix("/backone?currentPath=" + encodeURIComponent(currentFolderPath));

    fetch(url)
        .then(res => res.json())
        .then(data => {
            // 🔥 关键修正：必须把计算好的 targetParentPath 传给渲染器，锁死路径状态！
            displayFileList(data.files, targetParentPath);
            selectedFilesForDownload = [];
        })
        .catch(err => {
            console.error("后退失败，尝试降级刷新列表", err);
            getFiles();
        });
}

function creatfold(folderName) {
    // 🔥 加上当前目录 currentFolderPath
    var urltocall = addPrefix("/uploadaddFold?foldname=" + folderName + "&currentPath=" + encodeURIComponent(currentFolderPath));

    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function () {
        if (this.readyState == 4) {
            if (this.status == 200) {
                getFiles();
                showAlert("文件夹 '" + folderName + "' 创建成功", true);
            } else {
                showAlert("创建文件夹失败: " + this.responseText, false);
            }
        }
    };
    xmlhttp.open("GET", urltocall, true);
    xmlhttp.send();
}

window.selectFilesAndUploadButton = {
    onclick: function () {
        var uploadForm = document.createElement('input');
        uploadForm.type = 'file';
        uploadForm.multiple = true;
        uploadForm.onchange = function () {
            if (this.files.length > 0) {
                showModal();
                startUpload(this.files);
            }
        };
        document.body.appendChild(uploadForm);
        uploadForm.click();
    }
};

// 你原版结构 + 新版路径适配 100% 还原
window.createfile = {
    onclick: function () {
        document.getElementById('fileNameInput').value = '';
        document.getElementById('fileModal').style.display = 'block';

        const subBtn = document.getElementById('confirmCreateFile');
        subBtn.onclick = function () {
            let name = document.getElementById('fileNameInput').value.trim();
            if (!name) return;

            // ============= 只改这里：新版带目录请求 =============
            let url = addPrefix("/createFile?filename=" + encodeURIComponent(name) + "&currentPath=" + encodeURIComponent(currentFolderPath));

            let xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function () {
                if (this.readyState === 4) {
                    if (this.status === 200) {
                        getFiles();
                        showAlert(`文件${name}创建成功`, true);
                    } else {
                        showAlert('创建失败：' + this.responseText, false);
                    }
                }
            };
            xhr.open('GET', url, true);
            xhr.send();

            document.getElementById('fileModal').style.display = 'none';
        };
    }
};

window.createFolderBtn = {
    onclick: function () {
        document.getElementById('foldNameInput').value = '';
        document.getElementById('foldModal').style.display = 'block';

        const btn = document.getElementById('confirmCreate');
        btn.onclick = function () {
            var foldName = document.getElementById('foldNameInput').value.trim();
            if (!foldName) return;
            creatfold(foldName);
            document.getElementById('foldModal').style.display = 'none';
        };
    }
};

function showModal() {
    document.getElementById('uploadModal').style.display = 'block';
}

function closeModal() {
    document.getElementById('uploadModal').style.display = 'none';
}

function completeHandler(event) {
    closeModal();
    getFiles();
    showAlert('上传完成', true);
}

let lastLoaded = 0;
let lastTime = Date.now();

function progressHandler(event, index) {
    const loaded = event.loaded;
    const total = event.total;

    const loadedStr = bytesToSize(loaded);
    const totalStr = bytesToSize(total);

    const now = Date.now();
    const timeDiff = (now - lastTime) / 1000;
    let speedStr = "0 B/s";
    if (timeDiff > 0.1 && loaded > lastLoaded) {
        const speedBytes = (loaded - lastLoaded) / timeDiff;
        speedStr = bytesToSize(speedBytes) + "/s";
    }
    lastLoaded = loaded;
    lastTime = now;

    document.getElementById("modalProgressBar").value = Math.round((loaded / total) * 100);

    if (loaded >= total) {
        document.getElementById("modalStatus").innerHTML =
            `正在写入文件 (${loadedStr} / ${totalStr}) | 速度: ${speedStr} | ${uploadSuccessCount}/${uploadTotalCount}`;
    } else {
        document.getElementById("modalStatus").innerHTML =
            `${loadedStr} / ${totalStr} | 速度: ${speedStr} | ${uploadSuccessCount}/${uploadTotalCount}`;
    }
}

let uploadTotalCount = 0;
let uploadSuccessCount = 0;
let uploadFileList = [];

function startUpload(files) {
    uploadFileList = Array.from(files);
    uploadTotalCount = files.length;
    uploadSuccessCount = 0;
    uploadNextFile();
}

let uploadXHR = null;

function uploadNextFile() {
    if (uploadFileList.length === 0) {
        completeHandler();
        return;
    }

    let file = uploadFileList.shift();
    uploadXHR = new XMLHttpRequest();

    uploadXHR.upload.addEventListener("progress", function (event) {
        progressHandler(event, 1);
    }, false);

    uploadXHR.onload = function () {
        if (this.status == 200) {
            uploadSuccessCount++;
        }
        uploadNextFile();
    };

    // 🔥 这里加上当前目录路径，后端才能知道上传到哪里
    const url = addPrefix("/uploadAll?dir=" + encodeURIComponent(currentFolderPath));

    uploadXHR.open("POST", url, true);
    uploadXHR.setRequestHeader("File-Name", encodeURIComponent(file.name));
    uploadXHR.send(file);
}

function cancelUpload() {
    if (uploadXHR) {
        uploadXHR.abort();
        uploadXHR = null;
    }
    uploadFileList = [];
    closeModal();
}

function errorHandler(event) {
    closeModal();
}

function abortHandler(event) {
    closeModal();
}

document.querySelector('.containerlist').addEventListener('contextmenu', function (e) {
    e.preventDefault();
    const target = e.target.closest('[data-filepath]');
    const filePath = target ? target.dataset.filepath : '';
    showCustomContextMenu(e.clientX, e.clientY, filePath);
});

// 右键菜单动态定位：先加入DOM测量尺寸，再根据可用空间决定显示方向
// 下方空间不足则向上展开，右侧空间不足则向左展开，并保持 10px 边距
function positionContextMenu(menu, x, y) {
    document.body.appendChild(menu);
    const menuWidth = menu.offsetWidth;
    const menuHeight = menu.offsetHeight;
    const windowWidth = window.innerWidth;
    const windowHeight = window.innerHeight;

    let finalX = x;
    let finalY = y;

    if (x + menuWidth > windowWidth) {
        finalX = windowWidth - menuWidth - 10;
    }
    if (y + menuHeight > windowHeight) {
        finalY = windowHeight - menuHeight - 10;
    }
    if (finalX < 10) finalX = 10;
    if (finalY < 10) finalY = 10;

    menu.style.left = finalX + 'px';
    menu.style.top = finalY + 'px';
}

function displayFileList(files, currentPath) {
    totalFilesCount = files.length;

    // 1. 路径严格同步，防止 zip 渲染时把路径洗成 undefined
    if (currentPath) {
        currentFolderPath = currentPath;
    }

    let directoryUpdated = false;
    if (!directoryUpdated) {
        const pathElem = document.getElementById('path');
        if (pathElem) {
            let pathHtml = "当前目录：";
            pathHtml += `<span onclick="backRoot()">.</span> `;

            if (currentFolderPath === "/") {
                pathHtml += "/";
            } else {
                const parts = currentFolderPath.split('/').filter(p => p);
                pathHtml += `/`;

                let fullPath = "";
                parts.forEach((part, index) => {
                    fullPath += "/" + part;

                    if (zipRootFullPath && fullPath.startsWith(zipRootFullPath)) {
                        pathHtml += `<span onclick="currentFolderPath='${fullPath}';renderZipCurrentLevel();">${part}</span>`;
                    } else {
                        pathHtml += `<span onclick="lookthis('${fullPath}')">${part}</span>`;
                    }

                    if (index !== parts.length - 1) {
                        pathHtml += " / ";
                    }
                });
            }
            pathElem.innerHTML = pathHtml;
        }
        directoryUpdated = true;
    }

    const fileListElement = document.getElementById('fileList');
    if (!fileListElement) return;
    fileListElement.innerHTML = '';

    // 如果为空的处理
    if (!files || files.length === 0) {
        fileListElement.innerHTML = '<li class="empty-tip" style="text-align:center; padding:20px;">当前文件夹为空</li>';
    } else {
        files.forEach(item => {
            const listItem = document.createElement('li');
            listItem.className = 'file-item';

            const iconContainer = document.createElement('div');
            iconContainer.className = 'icon-container';

            // 绑定 item.filepath 属性，确保跟你在 renderZipCurrentLevel 里放入 map 的虚拟路径对齐
            iconContainer.setAttribute('data-filepath', item.filepath);

            // 保持原本的选中状态渲染
            if (selectedFilesForDownload.includes(item.filepath)) {
                iconContainer.classList.add('selected');
            }

            // =========================================================
            // ⚡【零延迟还原】：原生的 click 和 dblclick 各自独立工作
            // =========================================================

            // 单击：没有任何延迟，按下去直接触发多选/变色
            iconContainer.addEventListener('click', function (e) {
                e.stopPropagation(); // 阻止冒泡
                const path = this.getAttribute('data-filepath');

                if (this.classList.contains('selected')) {
                    const index = selectedFilesForDownload.indexOf(path);
                    if (index > -1) {
                        selectedFilesForDownload.splice(index, 1);
                    }
                    this.classList.remove('selected');
                } else {
                    selectedFilesForDownload.push(path);
                    this.classList.add('selected');
                }
                updateFileCountsDisplay();
            });

            // 双击：直接使用原生 dblclick 触发，不再被 setTimeout 阻塞，解决进 zip 打开失败问题
            iconContainer.addEventListener('dblclick', function (e) {
                e.stopPropagation();
                e.preventDefault();
                const path = this.getAttribute('data-filepath');
                const fileName = path.split('/').pop();
                const fileExtension = fileName.split('.').pop().toLowerCase();

                if (item.filesize == -1) {
                    // 如果在虚拟 zip 路径中，双击文件夹走前端 zip 渲染跳转
                    if (zipRootFullPath && path.startsWith(zipRootFullPath)) {
                        jumpInZip(path);
                        return;
                    }
                    lookthis(path);
                } else if (fileExtension === 'lua') {
                    runLuaFile(path);
                } else if (fileExtension === 'zip') {
                    enterZipDir(path); // 👈 纯净还原：原生双击事件直接触发进入 zip
                } else if (allowedExtensionsForModify.includes(fileExtension)) {
                    modify(path);
                } else if (allowedExtensionsForModifyPicture.includes(fileExtension)) {
                    lookPic(path);
                }
            });

            // 移动端长按支持保持不动
            let longPressTimer;
            iconContainer.addEventListener('touchstart', function (e) {
                longPressTimer = setTimeout(function () {
                    e.preventDefault();
                    showCustomContextMenu(e.touches[0].clientX, e.touches[0].clientY, this.getAttribute('data-filepath'));
                }, 500)
            });

            iconContainer.addEventListener('touchend', function () {
                clearTimeout(longPressTimer);
            });

            const iconSpan = document.createElement('span');
            iconSpan.classList.add('icon');
            const filenameSpan = document.createElement('span');
            filenameSpan.className = 'filename';
            const fileSizeSpan = document.createElement('span');

            const fullName = item.filepath.split('/').pop();
            filenameSpan.textContent = fullName;
            filenameSpan.title = fullName;
            iconSpan.title = fullName;

            if (item.filesize !== -1) {
                filenameSpan.textContent = fullName;
            }

            if (item.filesize == -1) {
                iconSpan.classList.add('icon-wenjianjia');
            } else {
                let ext = '';
                const dotIndex = fullName.lastIndexOf('.');
                if (dotIndex > 0) {
                    ext = fullName.slice(dotIndex + 1).toLowerCase();
                }
                const iconClass = extIconMap[ext] || extIconMap.default;
                iconSpan.classList.add(iconClass);
                fileSizeSpan.textContent = bytesToSize(parseInt(item.filesize));
            }

            iconContainer.appendChild(iconSpan);
            iconContainer.appendChild(filenameSpan);

            if (item.filesize != "-1") {
                iconContainer.appendChild(fileSizeSpan);
            }

            listItem.appendChild(iconContainer);
            fileListElement.appendChild(listItem);
        });
    }
    updateFileCountsDisplay();
}

function handleItemClick(event) {
    const filePath = event.current.getAttribute('data-filepath');

    if (event.current.classList.contains('selected')) {
        const index = selectedFilesForDownload.indexOf(filePath);
        if (index > -1) {
            selectedFilesForDownload.splice(index, 1);
        }
    } else {
        selectedFilesForDownload.push(filePath);
    }
    event.current.classList.toggle('selected');
    updateFileCountsDisplay();
}

function clearClickedOutside(event) {
    if (event.target.closest('.custom-context-menu') ||
        event.target.closest('#deleteConfirmModal') ||
        event.target.closest('#compressModal') ||
        event.target.closest('#extractModal')) {
        return;
    }
    if (!event.target.closest('.icon-container')) {
        selectedFilesForDownload = [];
        updateFileCountsDisplay();
        document.querySelectorAll('.icon-container.selected').forEach(element => {
            element.classList.remove('selected');
        });
    }
}

function showCustomContextMenu(x, y, filePath) {
    const oldMenu = document.querySelector('.custom-context-menu');
    if (oldMenu) oldMenu.remove();

    if (selectedFilesForDownload.length === 0) {
        const menu = document.createElement('div');
        menu.classList.add('custom-context-menu');
        menu.innerHTML = `
        <ul>
          <li onclick="selectFilesAndUploadButton.onclick()">
            <button class="top-icon-btn icon-btn">
              <i class="icon-upload-btn"></i>
              <span>上传文件</span>
            </button>
          </li>
          <li onclick="createfile.onclick()">
            <button class="top-icon-btn icon-btn">
              <i class="icon-chuangjianwenjian"></i>
              <span>新建文件</span>
            </button>
          </li>
          <li onclick="createFolderBtn.onclick()">
            <button class="top-icon-btn icon-btn">
              <i class="icon-chuangjianwenjianjia"></i>
              <span>新建文件夹</span>
            </button>
          </li>
        </ul>`;
        positionContextMenu(menu, x, y);
        return;
    }

    const clickedItem = document.querySelector(`[data-filepath="${filePath}"]`);
    const isFolder = clickedItem ? clickedItem.querySelector('.icon-wenjianjia') : false;
    const onlyOneFolder = selectedFilesForDownload.length === 1 && isFolder;
    const fileName = filePath.split('/').pop();
    const fileExtension = fileName.split('.').pop().toLowerCase();
    const menu = document.createElement('div');
    menu.classList.add('custom-context-menu');
    const isAllowedFileForModify = allowedExtensionsForModify.includes(fileExtension);
    let modifyOption = '';
    if (selectedFilesForDownload.length === 1 && isAllowedFileForModify) {
        modifyOption = `<li onclick="modify('${filePath}')">
    <button class="top-icon-btn icon-btn">
      <i class="icon-edit"></i>
      <span>编辑</span>
    </button>
  </li>`;
    }
    const isAllowedFileForModifyPicture = allowedExtensionsForModifyPicture.includes(fileExtension);
    let modifyOptionPicturn = '';
    if (selectedFilesForDownload.length === 1 && isAllowedFileForModifyPicture) {
        modifyOptionPicturn = `<li onclick="lookPic('${filePath}')">
    <button class="top-icon-btn icon-btn">
      <i class="icon-chakan"></i>
      <span>查看</span>
    </button>
  </li>`;
    }
    menu.innerHTML = `
<ul>
  ${selectedFilesForDownload.length === 1 && fileName.toLowerCase().endsWith('.lua') ? `
  <li onclick="runLuaFile('${filePath}')">
    <button class="top-icon-btn icon-btn" >
      <i class="icon-yunhang"></i>
      <span>运行</span>
    </button>
  </li>` : ''}

${selectedFilesForDownload.length >= 1 &&
            !selectedFilesForDownload.some(p => p.toLowerCase().endsWith('.zip')) ? `
<li onclick="showCompressModal()">
  <button class="top-icon-btn icon-btn">
    <i class="icon-wenjianyasuo"></i>
    <span>压缩</span>
  </button>
</li>` : ''}

${selectedFilesForDownload.length === 1 && fileName.toLowerCase().endsWith('.zip') ? `
  <li onclick="showExtractModal()">
    <button class="top-icon-btn icon-btn">
      <i class="icon-fileCompresswenjianjieyasuo"></i>
      <span>解压缩</span>
    </button>
  </li>
  <li onclick="lookZip('${filePath}')">
    <button class="top-icon-btn icon-btn">
      <i class="icon-chakan"></i>
      <span>查看</span>
    </button>
  </li>
` : ''}

  ${selectedFilesForDownload.length == 1 && isFolder ? `
  <li onclick="lookthis('${filePath}')">
    <button class="top-icon-btn icon-btn">
      <i class="icon-xinxidakai"></i>
      <span>打开</span>
    </button>
  </li>` : ''}
  
  ${modifyOption}
  ${modifyOptionPicturn}
  ${selectedFilesForDownload.length === 1 ? `
  <li onclick="copyFilePath('${filePath}')">
    <button class="top-icon-btn icon-btn">
        <i class="icon-fuzhilujing">
        </i><span>复制路径</span>
    </button>
  </li>` : ''}
  
  ${selectedFilesForDownload.length >= 1 && !isFolder ? `
  <li onclick="downloadSelectedFilesSequentially()">
    <button class="top-icon-btn icon-btn">
      <i class="icon-download"></i>
      <span>下载</span>
    </button>
  </li>` : ''}
  
  ${selectedFilesForDownload.length == 1 ? `
  <li onclick="showRenameModal('${filePath}')">
    <button class="top-icon-btn icon-btn">
      <i class="icon-zhongmingming"></i>
      <span>重命名</span>
    </button>
  </li>` : ''}
  
${selectedFilesForDownload.length >= 1 ? `
<li onclick="confirmDeleteMultipleFiles()">
  <button class="top-icon-btn icon-btn">
    <i class="icon-delete-fill"></i>
    <span>删除</span>
  </button>
</li>` : ''}

${onlyOneFolder ? `
<li onclick="forceDeleteSelected()">
  <button class="top-icon-btn icon-btn" style="color:red;font-weight:bold;">
    <i class="icon-delete-fill"></i>
    <span>强制删除</span>
  </button>
</li>` : ''}
</ul>
`;
    positionContextMenu(menu, x, y);
}

document.addEventListener('click', function () {
    const menu = document.querySelector('.custom-context-menu');
    if (menu) menu.remove();
});

window.onload = function () {
    const params = new URLSearchParams(window.location.search);
    const mode = params.get('mode');

    if (mode === 'upload') {
        fetch('/lookthis?lookthisPath=' + encodeURIComponent('/上传'))
            .then(res => res.json())
            .then(data => {
                displayFileList(data.files);
            });
    } else {
        storage();
    }

    document.body.addEventListener('click', clearClickedOutside);
};

function downloadSelectedFilesSequentially() {
    if (!selectedFilesForDownload.length) {
        showAlert('请先选择文件', false);
        return;
    }

    // 🔥 核心：从已加载的文件列表里 直接拿大小，不再请求后端
    function getFileSizeByPath(path) {
        const item = document.querySelector(`[data-filepath="${path}"]`);
        if (!item) return 0;
        const sizeText = item.nextSibling?.textContent || item.querySelector('span:last-child')?.textContent || '0 B';
        return sizeTextToBytes(sizeText);
    }

    // 大小文字转字节
    function sizeTextToBytes(text) {
        const units = { B: 1, KB: 1024, MB: 1024 * 1024, GB: 1024 * 1024 * 1024 };
        const match = text.match(/([\d.]+)\s*([BKMG]B)/i);
        if (!match) return 0;
        return parseFloat(match[1]) * units[match[2].toUpperCase()];
    }

    const files = [...selectedFilesForDownload];
    let currentIndex = 0;
    let lastUpdateTime = 0;
    let lastLoadedBytes = 0;

    document.getElementById('downloadProgressModal').style.display = 'block';

    function next() {
        if (currentIndex >= files.length) {
            setTimeout(() => {
                document.getElementById('downloadProgressModal').style.display = 'none';
                showAlert('下载完成，共 ' + files.length + ' 个文件', true);
            }, 800);
            return;
        }

        const fullPath = files[currentIndex];
        const fileName = fullPath.split('/').pop();
        // ✅ 直接用已有的大小！！！
        const totalBytes = getFileSizeByPath(fullPath);

        let url = '/download?path=' + encodeURIComponent(fullPath);
        url = addPrefix(url);

        document.getElementById('downloadTitle').textContent = '下载中 (' + (currentIndex + 1) + '/' + files.length + ')';
        document.getElementById('downloadFileName').textContent = fileName;
        document.getElementById('downloadProgressBar').value = 0;
        document.getElementById('downloadPercent').textContent = '0%';
        document.getElementById('downloadSize').textContent = `0 B / ${bytesToSize(totalBytes)}`;
        document.getElementById('downloadSpeed').textContent = '0 B/s';

        lastUpdateTime = Date.now();
        lastLoadedBytes = 0;
        downloadAbortController = new AbortController();

        fetch(url, { signal: downloadAbortController.signal })
            .then(response => {
                const reader = response.body.getReader();
                const chunks = [];
                let loaded = 0;

                function pump() {
                    return reader.read().then(({ done, value }) => {
                        if (done) {
                            const blob = new Blob(chunks);
                            const a = document.createElement('a');
                            a.href = URL.createObjectURL(blob);
                            a.download = fileName;
                            document.body.appendChild(a);
                            a.click();
                            document.body.removeChild(a);
                            URL.revokeObjectURL(a.href);

                            currentIndex++;
                            setTimeout(next, 300);
                            return;
                        }

                        chunks.push(value);
                        loaded += value.length;

                        const now = Date.now();
                        const timeDiff = (now - lastUpdateTime) / 1000;

                        // 每 0.3 秒更新一次速度（更稳定不跳变）
                        if (timeDiff >= 0.3 && loaded > lastLoadedBytes) {
                            const bytesDiff = loaded - lastLoadedBytes;
                            const speedBytesPerSecond = bytesDiff / timeDiff; // 🔥 正确每秒速度
                            document.getElementById('downloadSpeed').textContent = bytesToSize(speedBytesPerSecond) + '/s';
                            lastUpdateTime = now;
                            lastLoadedBytes = loaded;
                        }

                        // 进度
                        let percent = 0;
                        if (totalBytes > 0) {
                            percent = Math.min(100, Math.round((loaded / totalBytes) * 100));
                            document.getElementById('downloadProgressBar').value = percent;
                            document.getElementById('downloadPercent').textContent = percent + '%';
                        } else {
                            document.getElementById('downloadPercent').textContent = '...';
                        }

                        document.getElementById('downloadSize').textContent = `${bytesToSize(loaded)} / ${bytesToSize(totalBytes)}`;
                        return pump();
                    });
                }
                return pump();
            })
            .catch(error => {
                if (error.name === 'AbortError') return;
                showAlert('下载 ' + fileName + ' 失败', false);
                currentIndex++;
                setTimeout(next, 300);
            });
    }
    next();
}

function updateFileCountsDisplay() {
    const filenumElem = document.getElementById('filenum');
    if (filenumElem) {
        filenumElem.innerText = "文件：" + `${selectedFilesForDownload.length} / ${totalFilesCount}`;
    }
}

function showRenameModal(fullFilePath) {
    const renameBtn = document.getElementById('confirmRename');
    if (renameBtn) {
        renameBtn.onclick = null;
        renameBtn.onclick = applyRename;
    }
    const fileName = fullFilePath.split('/').pop();
    const renameInput = document.getElementById('renameInput');
    const hiddenFilePath = document.getElementById('hiddenFilePath');
    if (renameInput) renameInput.value = fileName;
    if (hiddenFilePath) hiddenFilePath.value = fullFilePath;
    const modal = document.getElementById('renameModal');
    if (modal) modal.style.display = 'block';
}

document.getElementById('cancelCreateFile').onclick = () => {
    document.getElementById('fileModal').style.display = 'none';
}

// 原有按钮关闭逻辑保留 + 新增 点击遮罩关闭全部弹窗
window.addEventListener('click', function (event) {
    const target = event.target;

    // 1. 点击弹窗外层遮罩 统一关闭
    document.querySelectorAll('.modal').forEach(modal => {
        if (target === modal) {
            const id = modal.id;
            switch (id) {
                case 'uploadModal': cancelUpload(); break;
                case 'downloadProgressModal': cancelDownload(); break;
                case 'modifyModal': closeModalEdit('modifyModal'); break;
                case 'myModal': modal.style.display = 'none'; break;
                default: modal.style.display = 'none';
            }
        }
    });

    // 2. 原有按钮/指定元素关闭逻辑 完全保留
    const modal = document.getElementById('foldModal');
    if (target == modal || target.id === 'cancelCreate') {
        modal.style.display = 'none';
    }

    const renameModal = document.getElementById('renameModal');
    if (target === renameModal || target.id === 'cancelRename') {
        renameModal.style.display = 'none';
    }

    const deleteModal = document.getElementById('deleteConfirmModal');
    if (target === deleteModal || target.id === 'cancelDelete') {
        deleteModal.style.display = 'none';
    }

    const downloadModal = document.getElementById('downloadProgressModal');
    if (target == downloadModal) {
        cancelDownload();
    }

    const modifyModal = document.getElementById('modifyModal');
    if (target == modifyModal) {
        closeModalEdit('modifyModal');
    }

    const uploadModal = document.getElementById('uploadModal');
    if (target == uploadModal) {
        cancelUpload();
    }
    const fileModal = document.getElementById('fileModal');
    if (target == fileModal || target.id === 'cancelCreateFile') {
        fileModal.style.display = 'none';
    }
    const compressModal = document.getElementById('compressModal');
    if (target == compressModal) {
        compressModal.style.display = 'none';
    }

    const extractModal = document.getElementById('extractModal');
    if (target == extractModal) {
        extractModal.style.display = 'none';
    }
});

function showAlert(message, isSuccess) {
    var notificationModal = document.getElementById('notificationModal');
    if (!notificationModal) {
        alert(message);
        return;
    }
    var notificationMessage = document.querySelector('.notification-message');
    notificationMessage.textContent = message;
    notificationMessage.style.color = isSuccess ? 'green' : 'red';
    notificationModal.style.display = 'block';

    const closeBtn = document.getElementById('closeNotification');
    if (closeBtn) {
        closeBtn.onclick = function () {
            notificationModal.style.display = 'none';
        };
    }
}

function renameFile(oldFilePath, newName) {
    // 🔥 直接传完整路径给后端，最安全！
    const url = addPrefix(
        "/renameFile?" +
        "oldPath=" + encodeURIComponent(oldFilePath) +
        "&newName=" + encodeURIComponent(newName)
    );

    const xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState == 4) {
            if (this.status == 200 && this.responseText == "OK") {
                showAlert("重命名成功", true);
                getFiles();
            } else {
                showAlert("重命名失败", false);
            }
        }
    };
    xhr.open("GET", url, true);
    xhr.send();
}

function hideModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) modal.style.display = 'none';
}

function applyRename() {
    const newName = document.getElementById('renameInput').value.trim();
    const oldPath = document.getElementById('hiddenFilePath').value;
    if (newName) {
        renameFile(oldPath, newName);
        hideModal('renameModal');
    } else {
        alert('文件名不能为空！');
    }
}

var saveHandler = function (filename) {
    return function () {
        var content = document.getElementById('textareaContent').value;
        saveModifiedContent(filename, content);
    };
};

function modify(filename) {
    var urltocall = addPrefix("/modify?filename=" + encodeURIComponent(filename));

    const modalFilename = document.getElementById('modalFilename');
    if (modalFilename) modalFilename.innerText = filename;
    const textArea = document.getElementById('textareaContent');
    if (textArea) textArea.value = '正在从存储读取数据...';
    const modal = document.getElementById('modifyModal');
    if (modal) modal.style.display = 'block';

    const saveBtn = document.getElementById('saveButton');
    if (saveBtn) {
        const newSaveBtn = saveBtn.cloneNode(true);
        saveBtn.parentNode.replaceChild(newSaveBtn, saveBtn);
        newSaveBtn.onclick = function () {
            var content = textArea ? textArea.value : '';
            saveModifiedContent(filename, content);
        };
    }

    const modalClose = document.getElementById('modalClose');
    if (modalClose) {
        modalClose.onclick = function () {
            closeModalEdit('modifyModal');
        };
    }

    fetch(urltocall)
        .then(response => {
            if (!response.ok) throw new Error('读取失败，状态码: ' + response.status);
            return response.text();
        })
        .then(textContent => {
            if (textArea) textArea.value = textContent;
        })
        .catch(error => {
            if (textArea) textArea.value = '';
            alert("读取出错: " + error.message);
        });
}

function saveModifiedContent(filename, content) {
    const text = content;
    const txtpath = filename;
    const MAX_CHUNK_BYTES = 16384;
    const CHINESE_BYTE_PER_CHAR = 3;
    const MAX_CHINESE_PER_CHUNK = Math.floor(MAX_CHUNK_BYTES / CHINESE_BYTE_PER_CHAR);

    let currentChunk = 0;
    const totalChars = text.length;
    const totalChunks = Math.ceil(totalChars / MAX_CHINESE_PER_CHUNK);

    let retry = 0, maxRetry = 3;

    if (totalChars === 0) {
        showAlert('没有内容可保存', false);
        return;
    }

    const titleElem = document.getElementById('modalFilename');
    const originalTitle = filename;

    const sendChunk = () => {
        if (currentChunk >= totalChunks) {
            if (titleElem) titleElem.innerText = originalTitle;
            showAlert('保存成功', true);
            getFiles();
            hideModal('modifyModal');
            return;
        }

        const startChar = currentChunk * MAX_CHINESE_PER_CHUNK;
        const endChar = Math.min(startChar + MAX_CHINESE_PER_CHUNK, totalChars);
        let chunkData = text.substring(startChar, endChar);

        const encoder = new TextEncoder();
        const actualBytes = encoder.encode(chunkData).length;

        if (actualBytes > MAX_CHUNK_BYTES) {
            let safeEnd = startChar + 1;
            while (safeEnd <= endChar) {
                const safeData = text.substring(startChar, safeEnd);
                if (encoder.encode(safeData).length > MAX_CHUNK_BYTES) {
                    safeEnd--;
                    break;
                }
                safeEnd++;
            }
            chunkData = text.substring(startChar, safeEnd);
        }

        if (titleElem) {
            titleElem.innerText = `正在保存 (${currentChunk + 1}/${totalChunks})...`;
        }

        fetch(addPrefix('/editTxt'), {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                txtpath,
                totalChunks,
                currentChunk,
                con: chunkData,
                charCount: chunkData.length,
                expectedBytes: encoder.encode(chunkData).length
            }),
            timeout: 5000
        })
            .then(response => response.ok ? response.text() : Promise.reject(`HTTP错误: ${response.status}`))
            .then(responseText => {
                if (responseText === "OK") {
                    retry = 0;
                    currentChunk++;
                    sendChunk();
                } else {
                    throw `服务器响应异常: ${responseText}`;
                }
            })
            .catch(error => {
                console.error(`保存失败:`, error);
                if (retry < maxRetry) {
                    retry++;
                    if (titleElem) titleElem.innerText = `重试中(${retry})...`;
                    setTimeout(sendChunk, 1000 * retry);
                } else if (confirm('保存失败，是否重新尝试？')) {
                    currentChunk = retry = 0;
                    sendChunk();
                }
            });
    };

    sendChunk();
}

function closeModalEdit(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) modal.style.display = 'none';
    selectedFilesForDownload = [];
}

function lookPic(path) {
    var imgElement = document.getElementById('displayedImage');
    var captionText = document.getElementById("caption");
    if (imgElement) imgElement.src = path;
    if (captionText) captionText.innerHTML = '';
    var modal = document.getElementById("myModal");
    if (modal) modal.style.display = "block";
}

function checkClick(event) {
    var modal = document.getElementById("myModal");
    if (event.target === modal) {
        modal.style.display = "none";
    }
}

function bytesToSize(bytes) {
    var sizes = ['B', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];
    if (bytes == 0) return '0 B';
    var i = parseInt(Math.floor(Math.log(bytes) / Math.log(1024)));
    var size = bytes / Math.pow(1024, i);
    if (sizes[i] !== 'B') {
        size = size.toFixed(2);
    }
    return size + ' ' + sizes[i];
}

function cancelDownload() {
    if (downloadAbortController) {
        downloadAbortController.abort();
        downloadAbortController = null;
    }
    document.getElementById('downloadProgressModal').style.display = 'none';
}

function createFile(fileName) {
    let url = addPrefix("/createFile?filename=" + encodeURIComponent(fileName));
    let xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        if (this.readyState === 4) {
            if (this.status === 200) {
                getFiles();
                showAlert(`文件${fileName}创建成功`, true);
            } else {
                showAlert('创建失败：' + this.responseText, false);
            }
        }
    };
    xhr.open('GET', url, true);
    xhr.send();
}

function runLuaFile(filePath) {
    const fileName = filePath.split('/').pop();

    fetch(addPrefix("/run_lua"), {
        method: 'POST',
        headers: { "Content-Type": "text/plain" },
        body: "run " + filePath,
    })
        .then(res => res.text())
        .then(text => {
            showAlert("运行成功：" + fileName + "\n\n结果：\n" + text, true);
        })
        .catch(err => {
            showAlert("运行失败：" + err.message, false);
        });
}

function showCompressModal() {
    document.getElementById("compressFilename").value = "";
    document.getElementById("compressModal").style.display = "block";
}

function showExtractModal() {
    document.getElementById("extractDirname").value = "";
    document.getElementById("extractModal").style.display = "block";
}

function startCompress() {
    let zipName = document.getElementById("compressFilename").value.trim();
    if (!zipName) return;
    if (!zipName.endsWith(".zip")) zipName += ".zip";

    document.getElementById("compressModal").style.display = "none";
    showAlert("压缩处理中...请勿退出", true);

    // =======================
    // ✅ 不编码！！！
    // =======================
    let param = "zipName=" + zipName + "&currentPath=" + currentFolderPath;

    // ✅ 文件也不编码！！！
    for (let i = 0; i < selectedFilesForDownload.length; i++) {
        param += "&filePath" + i + "=" + selectedFilesForDownload[i];
    }

    let xhr = new XMLHttpRequest();
    xhr.open("POST", addPrefix("/compress"), true);
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

    xhr.onload = function () {
        if (xhr.status === 200) {
            getFiles();
            showAlert("压缩成功", true);
        } else {
            showAlert("压缩失败", false);
        }
    };

    xhr.send(param);
}

function startExtract() {
    if (selectedFilesForDownload.length !== 1) {
        showAlert("请选择一个 ZIP 文件", false);
        return;
    }

    let zipPath = selectedFilesForDownload[0];
    let outDir = document.getElementById("extractDirname").value.trim();

    // 拼接当前目录 + 输入名称
    if (!outDir) {
        outDir = currentFolderPath;
    } else {
        outDir = currentFolderPath + "/" + outDir;
    }

    document.getElementById("extractModal").style.display = "none";
    showAlert("解压处理中...请勿退出", true);

    let url = addPrefix("/extract?zipPath=" + encodeURIComponent(zipPath) + "&outDir=" + encodeURIComponent(outDir));

    let xhr = new XMLHttpRequest();
    xhr.open("GET", url, true);
    // 移除 timeout 相关代码

    xhr.onload = function () {
        if (xhr.status === 200) {
            getFiles();
            showAlert("解压成功", true);
        } else {
            showAlert("解压失败", false);
        }
    };

    // 移除超时监听
    xhr.send();
}

function lookZip(filePath) {
    let url = addPrefix("/zip_list?path=" + encodeURIComponent(filePath));
    fetch(url)
        .then(res => res.json())
        .then(data => {
            let fileList = data.join("\n");
            showAlert("ZIP 内含文件：\n\n" + fileList, true);
        })
        .catch(err => {
            showAlert("查看 ZIP 失败", false);
        });
}

// ==========================
// 进入 ZIP
// ==========================
function enterZipDir(zipFilePath) {
    zipPrevRealPath = currentFolderPath;
    zipRootFullPath = zipFilePath;
    currentFolderPath = zipFilePath;

    fetch(addPrefix("/zip_list?path=" + encodeURIComponent(zipFilePath)))
        .then(res => res.json())
        .then(data => {
            zipInnerDataCache = data || [];
            renderZipCurrentLevel();
        })
        .catch(err => showAlert("打开ZIP失败", false));
}

function renderZipCurrentLevel() {
    const map = {};
    const relPath = currentFolderPath.replace(zipRootFullPath, "");
    const prefix = relPath.replace(/^\/+/, '') + (relPath ? '/' : '');

    zipInnerDataCache.forEach(item => {
        const name = item.name || "";
        const size = item.size || 0;
        if (!name.startsWith(prefix)) return;

        const rest = name.slice(prefix.length);
        if (!rest) return;

        const parts = rest.split('/');
        const firstName = parts[0];

        if (parts.length === 1) {
            map[firstName] = { filepath: currentFolderPath + "/" + firstName, filesize: size };
        } else {
            map[firstName] = { filepath: currentFolderPath + "/" + firstName, filesize: -1 };
        }
    });

    const pathElem = document.getElementById("path");
    if (pathElem) {
        let pathHtml = "当前目录：<span onclick='backRoot()'>.</span> /";

        const realParts = zipPrevRealPath.split('/').filter(Boolean);
        let tempReal = '';
        realParts.forEach(p => {
            tempReal += '/' + p;
            // ==============================
            // ✅ ZIP 前面的目录：直接发请求，不拦截！
            // ==============================
            pathHtml += `<span onclick="lookthis('${tempReal}')">${p}</span> /`;
        });

        const zipName = zipRootFullPath.split('/').pop();
        pathHtml += `<span onclick="currentFolderPath='${zipRootFullPath}';renderZipCurrentLevel();">${zipName}</span>`;

        // ==============================
        // ✅ ZIP 后面的目录：内部处理
        // ==============================
        const innerParts = relPath.split('/').filter(Boolean);
        let tempPath = zipRootFullPath;
        innerParts.forEach((p) => {
            tempPath += '/' + p;
            pathHtml += ` / <span onclick="currentFolderPath='${tempPath}';renderZipCurrentLevel();">${p}</span>`;
        });

        pathElem.innerHTML = pathHtml;
    }

    displayFileList(Object.values(map));
    selectedFilesForDownload = [];
}

// ==========================
// 路径点击跳转（ZIP专用）
// ==========================
function jumpInZip(path) {
    currentFolderPath = path;
    renderZipCurrentLevel();
}
// 强制删除 - 调用 deleteFileAll 接口
function forceDeleteSelected() {
    if (!selectedFilesForDownload.length) return;

    // 🔥 用你项目统一的自定义弹窗，不再用原生 confirm
    const delBtn = document.getElementById('confirmDelete');
    if (delBtn) {
        delBtn.onclick = null;
        delBtn.onclick = function () {
            document.getElementById('deleteConfirmModal').style.display = 'none';

            showAlert('强制删除中...', true);
            let count = 0;
            selectedFilesForDownload.forEach(path => {
                fetch(addPrefix('/deleteFileAll?deletePath=' + encodeURIComponent(path)))
                    .then(() => {
                        count++;
                        if (count === selectedFilesForDownload.length) {
                            selectedFilesForDownload = [];
                            updateFileCountsDisplay();
                            getFiles();
                            showAlert('删除完成', true);
                        }
                    })
                    .catch(() => showAlert('删除失败', false));
            });
        };
    }

    // 显示你现有的删除确认弹窗
    document.getElementById('deleteConfirmModal').style.display = 'block';
}
// 🔥 专用：点击面包屑路径跳转（新接口）
function selectPath(targetPath) {
    // 清空选择，避免误操作
    selectedFilesForDownload = [];
    updateFileCountsDisplay();

    // 用新接口 /select_path
    const url = addPrefix("/select_path?path=" + encodeURIComponent(targetPath));

    fetch(url)
        .then(res => res.json())
        .then(data => {
            displayFileList(data.files);
        })
        .catch(err => {
            console.error("路径跳转失败", err);
        });
}
// 全局安全关闭右键菜单（不影响任何操作，100%生效）
document.addEventListener('mousedown', function (e) {
    const menu = document.querySelector('.custom-context-menu');
    if (!menu) return;

    // 不是点菜单 → 立即关闭
    if (!e.target.closest('.custom-context-menu')) {
        menu.remove();
    }
}, true);
// 正确：拼接 当前目录 + 文件名
function copyFilePath(filePath) {
    // 最终路径 = 直接用你点击的 filePath（已经是拼接好的完整路径）
    const fullPath = filePath;

    // 复制到剪贴板
    const input = document.createElement('textarea');
    input.value = fullPath;
    document.body.appendChild(input);
    input.select();
    document.execCommand('copy');
    document.body.removeChild(input);

    // 提示（和你项目一模一样）
    showAlert('路径已复制：\n' + fullPath, true);
}