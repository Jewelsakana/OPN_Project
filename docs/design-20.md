### 当前任务  
实现XML编辑命令  
**注意**：  
1. 实现命令时需要从CommandParser.h加入enum字段注册宏调用，注册策略还需要在CommandParserstratery.h/.cpp中加入解析器，并在自身的Command类自注册编辑器命令工厂以及在CommandFactory类中添加自注册宏  
2. 最好不要看pugiconfig.hpp和pugixml.hpp这两个类，这是外部库，很大。  
**任务**：  
1. `insert-before` - 插入元素：  
* 功能：在目标元素前（同级）插入一个新元素  
* 格式： insert-before <tagName> <newId> <targetId> ["text"]  
* 参数说明：  
  * tagName:新插入的元素标签名  
  * newId:新元素的唯一ID,不可与已有元素重复  
  * targetId:目标元素的ID，新元素将被插入到该元素前  
  * text:可选，新元素的文本内容  
* 异常处理：  
  * newId：已存在：提示"元素ID已存在: [newId]"  
  * targetId：提示"目标元素不存在: [targetId]"  
  * 尝试在根元素前插入：提示"不能在根元素前插入元素"  
2. `append-child` - 追加子元素  
* 功能：在某元素内追加一个子元素(作为最后一个子元素)。
* 格式：append-child <tagName> <newId> <parentId> ["text"]  
* 参数说明：  
  * tagName:要追加的子元素标签名    
  * newId:子元素ID，需唯一    
  * targetId:父元素ID    
  * text:可选，子元素的文本内容  
* 异常说明：  
   * parentId无效：提示"父元素不存在: [parentId]"  
   * newId重复：提示"元素ID已存在: [newId]"  
3.  `edit-id` - 修改元素ID  
* 功能： 修改某个元素的ID
* 格式：edit-id <oldId> <newId>  
* 参数说明：  
   * oldId:原始ID，必须存在  
   * newId:目标ID，必须未占用  
* 异常处理：  
   * oldId不存在：提示"元素不存在: [oldId]"  
   * newId已被占用：提示"目标ID已存在: [newId]"  
   * 尝试修改根元素ID：提示"不建议修改根元素ID"  
4. `edit-text` - 修改元素文本  
* 功能：修改某个元素的文本  
* 格式：edit-text <elementId> ["text"]  
* 参数说明：  
  * elementId: 元素的ID  
  * "text":新文本内容(可选)，若为空字符串或省略则清空原内容  
* 异常处理：  
  * elementId不存在：提示"元素不存在: [elementId]"  
5.  `delete` - 删除元素  
* 功能：删除指定ID的元素(包括其所有子元素)  
* 异常处理：  
   * elementId不存在：提示"元素不存在: [elementId]"     
   * 尝试删除根元素：提示"不能删除根元素"
6. `xml-tree` - 显示XML树形结构
先暂时不实现，等后续再实现  
**要求**  
1. 需要实现自动化测试   
2. 需要将所完成的功能写入SUMMARY_2.md中  
3. 根标签只能有一个，子标签可以有多个，且必须成对存在    
4. 在实现 XML 命令时，请确保 undo() 方法能够恢复修改前的状态（例如保存旧 ID、旧文本或被删除的子树快照）   
5. 所有增删改操作完成后，必须同步更新内部维护的 id -> element 映射表，以保证后续命令的定位准确  
6. 可以完善相关XMLEngine类来实现具体的操作，命令只是调用这些函数  
7. 完成任务时清除相关编译的文件  
