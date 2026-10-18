# 上传源码

把本文件夹的内容作为仓库根目录。各源码树保留的上游 `.gitignore` 会忽略一些原本就需要交付的资源，所以首次添加请使用随包白名单：

```sh
git init
git --literal-pathspecs add -f --pathspec-from-file=SOURCE-PATHS.txt
git commit -m "Import preserved Chinese localization source trees and AF r21"
```

之后关联你创建的 GitHub 仓库并推送即可。ZIP 也可作为源码归档附件。不要上传整个旧工作区。

`SOURCE-PATHS.txt` 只列本次核对过的文件，避免把后来构建的缓存也一起加入。原许可证、版本映射与已知缺口说明都应提交。

仓库说明必须保留 5D3 r9 源码缺口，以及 7D/1200D 历史包的打包异常。此仓库是最后工作副本汇总，不是所有旧二进制均可精确复现的保证。
