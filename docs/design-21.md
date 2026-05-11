### 当前任务  
接着实现XML编辑命令和对workspace中的init,load,save命令底层实现的兼容  
**注意**：  
1. 实现命令时需要从CommandParser.h加入enum字段注册宏调用，注册策略还需要在CommandParserstratery.h/.cpp中加入解析器，并在自身的Command类自注册编辑器命令工厂以及在CommandFactory类中添加自注册宏  
2. 最好不要看pugiconfig.hpp和pugixml.hpp这两个类，这是外部库，很大。  
**任务**：  
1.  `xml-tree` - 显示XML树形结构  
* 格式：xml-tree [file]  
* 功能：以树形结构打印XML文件内容，展示元素的层级关系，属性和文本内容  
* 参数说明：  
   * 不指定参数，显示当前活动文件  
   * 'file':显示指定XML文件  
* 仅适用于XML编辑器  
* 输出格式要求：  
  * 使用树形字符(`├──`、`└──`、`│`)表示层级关系（可以去使用OutputSerive类中的函数，之前的命令有相关的函数实现）  
  * 显示元素的所有属性(包括id)  
  * 显示元素的文本内容(如果有)  
如：  
```xml  
<?xml version="1.0" encoding="UTF-8"?>  
<bookstore id="root">  
    <book id="book1" category="COOKING"> 
        <title id="title1" lang="en">Everyday Italian</title>  
    </book>  
</bookstore>  
```  
输出：   
```  
bookstore [id="root"]  
├── book [id="book1", category="COOKING"]  
│   └── title [id="title1", lang="en"]  
│       └── "Everyday Italian"  
```  
注意：这是显示类，不进入撤销栈  
2.  `init` - 创建新缓冲区  
* 格式：init <file> [with-log]  
* 已经存在一个init类命令，需要根据`file`的文件扩展名自动识别类型，从而创建对应的文件(.txt 创建纯文本文件，.xml 创建XML文件)  
如：  
创建XML文件(`init test.xml`):  
```xml  
<?xml version="1.0" encoding="UTF-8"?>  
<root id="root">  
</root>  
```   
* 注意：原来的init命令最终是交给FileCoordinator类来实现，因为需要实现创建XML文件。你可以选择在其上创建新函数，或者设置一个基类，然后派生出两个处理txt文件和xml文件的类  
3. `save`和`load`:  
这两个命令之前也已经实现过了，需要区分.txt和.xml的保存与加载，最终也是交给FileCoordinator类来实现的，所以同2一样考虑哪个方法更好  
4. 观察到workspace类中Editor类只有TextEditor类，为了适配新的XMlEditor类，作出相应的修改，打开什么样的文件，就能够调用相应的编辑器，之前实现过了Editor类工厂，请结合其一起使用，尽可能不要出现switch-case语句来1处理生成对应的Editor  
**要求**  
1. 需要实现自动化测试   
2. 需要将所完成的功能写入SUMMARY_2.md中  
3. 完成任务时清除相关编译的文件  
4.  `load` `save` `init`命令签名相同，但 .txt 与 .xml 底层实现分离  
5. 对于`append` `insert` `replace` `show` `delete <line:col> <len>` 这些txt文件的命令，应该对.xml文件提示不支持， `insert-before` `append-child` `edit-id` `edit-text` `xml-tree` `delete <elementId>`对于.txt文件应该提示不支持，不要直接在命令类加，个人觉得应该在解析命令的时候进行判断  
6. 可以考虑在 IEditor 接口中增加一个 supportsCommand(CommandType) 的方法，或者利用命令注册表中的元数据进行匹配。   
7. `xml-tree`可以使用OutputSerive类的函数或者在其中定义新的来实现  
8. `init`中生成的 XML 必须包含实验要求的 XML 声明（第一行）和带有 id="root" 的根节点，以确保新创建的文件能够立即被编辑逻辑识别。   
9.  `<?xml version="1.0" encoding="UTF-8" ?>` 必须写在首行(除非第一行是 `# log` 注释)  