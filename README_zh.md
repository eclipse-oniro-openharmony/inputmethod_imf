# inputmethod_imf

#### 介绍
输入法框架，主要作用是拉通应用和输入法，保证应用可以通过输入法进行文本输入

**图 1**  子系统架构图<a name="fig143011012341"></a>  
![](figures/subsystem_architecture_zh.png "子系统架构图")

#### 仓路径
/base/inputmethod/imf

## 目录

```
/base/inputmethod/imf
├── figures                              # 构架图
├── frameworks/inputmethod_ability       # 对输入法客户端提供的接口
├── frameworks/inputmethod_controller    # 对客户端提供的接口
├── interfaces/kits/js                   # 组件对外提供的接口代码
│   └── napi                             # 输入法框架napi接口
├── profile                              # 组件包含的系统服务的配置文件和进程的配置文件
├── services                             # 输入法框架服务
├── test                                 # 接口的Fuzz测试和js单元测试
└── unitest                              # 接口的单元测试
```

#### 框架代码介绍
输入法框架目前有四大模块，具体如下：

1.  应用客户端

路径：/base/inputmethod/imf/frameworks/inputmethod_controller

作用：实现应用和输入法框架服务交付，包括应用与输入法服务的绑定、应用对输入法的显示和隐藏请求等等

2.  输入法客户端

路径：/base/inputmethod/imf/frameworks/inputmethod_ability

作用：实现输入法框架服务与输入法交付的中间桥梁，包括监听输入法当前的状态等等

3.  输入法服务

路径：/base/inputmethod/imf/services

作用：作为输入法框架的核心，输入法的主要处理逻辑都是在这里完成

4.  输入法Js接口

路径：/base/inputmethod/imf/interfaces/kits/js

作用：暂时对外暴露的js接口，主要是留给输入法进行调用使用的

#### 说明

##### 接口说明

**表 1**   inputMethod开放的主要方法

<table><thead align="left"><tr id="row143351854201012"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p103351154121010"><a name="p103351154121010"></a><a name="p103351154121010"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p1033585416105"><a name="p1033585416105"></a><a name="p1033585416105"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>function getInputMethodController(): InputMethodController;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>获取客户端实例InputMethodController</p>
</td>
</tr>
<tr id="row13335054111018"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p12832214151418"><a name="p12832214151418"></a><a name="p12832214151418"></a>function getInputMethodSetting(): InputMethodSetting;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p3335145451011"><a name="p3335145451011"></a><a name="p3335145451011"></a>获取客户端设置实例InputMethodSetting</p>
</td>
</tr>
<tr id="row13335054111018"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p12832214151418"><a name="p12832214151418"></a><a name="p12832214151418"></a>function switchInputMethod(target: InputMethodProperty, callback: AsyncCallback&lt;boolean&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p3335145451011"><a name="p3335145451011"></a><a name="p3335145451011"></a>切换输入法，使用callback形式返回结果。参数个数为2，否则抛出异常</p>
</td>
</tr>
<tr id="row13335054111018"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p12832214151418"><a name="p12832214151418"></a><a name="p12832214151418"></a>function switchInputMethod(target: InputMethodProperty): Promise&lt;boolean&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p3335145451011"><a name="p3335145451011"></a><a name="p3335145451011"></a>切换输入法，使用promise形式返回结果。参数个数为1，否则抛出异常</p>
</td>
</tr>
</tbody>
</table>

##### 常量值

| 名称         | 参数类型 | 可读 | 可写 | 说明                     |
| ------------ | -------- | ---- | ---- | ------------------------ |
| MAX_TYPE_NUM | number   | 是   | 否   | 可支持的最大输入法个数。 |

##### 使用说明

```
// 导入模块
import inputmethod from '@ohos.inputmethod';

// 获取客户端实例
console.log('Get inputMethodController');
let inputMethodController = inputmethod.getInputMethodController();

// 获取客户端设置实例
console.log('Get inputMethodSetting');
let inputMethodSetting = inputmethod.getInputMethodSetting();

// 切换输入法callback
inputmethod.switchInputMethod({packageName:"com.example.kikakeyboard", methodId:"com.example.kikakeyboard"} ,(err,result) => {
    if (err) {
        console.info("switchInputMethod callback result---err: " + err.msg);
        return;
    }
    if (result) {
        console.info("Success to switchInputMethod.(callback)");
    } else {
        console.info("Failed to switchInputMethod.(callback)");
    }
});

// 切换输入法promise
await inputMethod.switchInputMethod({packageName:"com.example.kikakeyboard", methodId:"com.example.kikakeyboard"}).then((result) => {
    if (result) {
        console.info("Success to switchInputMethod.(promise)");
    } else {
        console.info("Failed to switchInputMethod.(promise)");
    }
}).catch((err) => {
    console.info("switchInputMethod promise err: " + err.msg);
});
```

**表 2**   InputMethodController开放的主要方法

<table><thead align="left"><tr id="row143351854201012"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p103351154121010"><a name="p103351154121010"></a><a name="p103351154121010"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p1033585416105"><a name="p1033585416105"></a><a name="p1033585416105"></a>描述</p>
</th>
</tr>
</thead>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>stopInput(callback: AsyncCallback&lt;boolean&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>隐藏输入法，使用callback形式返回结果。参数个数为1，否则抛出异常</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>stopInput(): Promise&lt;boolean&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>隐藏输入法，使用promise形式返回结果。参数个数为0，否则抛出异常</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>showSoftKeyboard(callback: AsyncCallback&lt;void&gt;);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>显示软键盘，使用callback形式返回结果。参数个数为1，否则抛出异常</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>stopInput(): Promise&lt;void&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>显示软键盘，使用promise形式返回结果。参数个数为0，否则抛出异常</p>
</td>
</tr><tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>hideSoftKeyboard(callback: AsyncCallback&lt;void&gt;);</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>隐藏软键盘，使用callback形式返回结果。参数个数为1，否则抛出异常</p>
</td>
</tr><tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>hideSoftKeyboard(): Promise&lt;void&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>隐藏软键盘，使用promise形式返回结果。参数个数为0，否则抛出异常</p>
</td>
</tr>
</tbody>
</table>


##### 使用说明

```
// 导入模块
import inputmethod from '@ohos.inputmethod';

console.log('Get inputMethodController');
let inputMethodController = inputmethod.getInputMethodController();

// 隐藏输入法callback
inputMethodController.stopInput((err, result) => {
    if (err) {
        console.error("stopInput callback result---err: " + err.msg);
        return;
    }
    if (result) {
        console.info("Success to stopInput.(callback)");
    } else {
        console.info("Failed to stopInput.(callback)");
    }
});

// 隐藏输入法promise
await inputMethodController.stopInput().then((result)=>{
    if (result) {
        console.info("Success to stopInput.(promise)");
    } else {
        console.info("Failed to stopInput.(promise)");
    }
}).catch((err) => {
    console.error("stopInput promise err: " + err.msg);
});

// 显示软键盘callback
inputMethodController.showSoftKeyboard((err) => {
    if (err) {
        console.error("showSoftKeyboard callback result---err: " + err.msg);
        return;
    }
    console.info("Success to showSoftKeyboard.(callback)");
});

// 显示软键盘promise
await inputMethodController.showSoftKeyboard().then(()=>{
    console.info("Success to showSoftKeyboard.(promise)");
}).catch((err) => {
    console.error("showSoftKeyboard promise err: " + err.msg);
});

// 隐藏软键盘callback
inputMethodController.hideSoftKeyboard((err) => {
    if (err) {
        console.error("hideSoftKeyboard callback result---err: " + err.msg);
        return;
    }
    console.info("Success to hideSoftKeyboard.(callback)");
});

// 隐藏软键盘promise
await inputMethodController.hideSoftKeyboard().then(()=>{
    console.info("Success to hideSoftKeyboard.(promise)");
}).catch((err) => {
    console.error("hideSoftKeyboard promise err: " + err.msg);
});

```

**表 3**   InputMethodSetting开放的主要方法

<table><thead align="left"><tr id="row143351854201012"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p103351154121010"><a name="p103351154121010"></a><a name="p103351154121010"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p1033585416105"><a name="p1033585416105"></a><a name="p1033585416105"></a>描述</p>
</th>
</tr>
</thead>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>listInputMethod(callback: AsyncCallback&lt;Array&lt;InputMethodProperty&gt;&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>查询已安装的输入法列表，使用callback形式返回结果。参数个数为1，否则抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>listInputMethod(): Promise&lt;Array&lt;InputMethodProperty&gt;&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>查询已安装的输入法列表，使用promise形式返回结果。参数个数为0，否则抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>displayOptionalInputMethod(callback: AsyncCallback&lt;void&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>显示输入法选择对话框，使用callback形式返回结果。参数个数为1，否则抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>displayOptionalInputMethod(): Promise&lt;void&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>显示输入法选择对话框，使用promise形式返回结果。参数个数为0，否则抛出异常。</p>
</td>
</tr>
</tbody>
</table>

##### 使用说明

```
// 导入模块
import inputmethod from '@ohos.inputmethod';

console.log('Get inputMethodSetting');
let inputMethodSetting = inputmethod.getInputMethodSetting();

// 查询已安装的输入法列表callback
inputMethodSetting.listInputMethod((err,data) => {
    if (err) {
        console.error("listInputMethod callback result---err: " + err.msg);
        return;
    }
    console.info("listInputMethod callback result---data: " + JSON.stringify(data));
 });

// 查询已安装的输入法列表promise
await inputMethodSetting.listInputMethod().then((data)=>{
    console.info("listInputMethod promise result---data: " + JSON.stringify(data));
}).catch((err) => {
    console.info("listInputMethod promise err:" + err.msg);
});

// 显示输入法选择对话框callback
inputMethodSetting.displayOptionalInputMethod((err) => {
    if (err) {
        console.error("displayOptionalInputMethod callback---err: " + err.msg);
        return;
    }
    console.info("displayOptionalInputMethod callback");
});
        
// 显示输入法选择对话框promise
await inputMethodSetting.displayOptionalInputMethod().then(()=>{
    console.info("displayOptionalInputMethod promise");
}).catch((err) => {
    console.info("listInputMethod promise err: " + err.msg);
});
```

**表 4**   inputMethodEngine开放的主要方法

<table><thead align="left"><tr id="row143351854201012"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p103351154121010"><a name="p103351154121010"></a><a name="p103351154121010"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p1033585416105"><a name="p1033585416105"></a><a name="p1033585416105"></a>描述</p>
</th>
</tr>
</thead>
<tbody><tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>function getInputMethodEngine(): InputMethodEngine;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>获取服务端实例</p>
</td>
</tr>
<tr id="row13335054111018"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p12832214151418"><a name="p12832214151418"></a><a name="p12832214151418"></a>function createKeyboardDelegate(): KeyboardDelegate;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p3335145451011"><a name="p3335145451011"></a><a name="p3335145451011"></a>获取客户端监听实例</p>
</td>
</tr>
</tbody>
</table>

**常量值**

| 名称                       | 参数类型 | 可读 | 可写 | 说明                   |
| -------------------------- | -------- | ---- | ---- | ---------------------- |
| ENTER_KEY_TYPE_UNSPECIFIED | number   | 是   | 否   | 无功能键。             |
| ENTER_KEY_TYPE_GO          | number   | 是   | 否   | “前往”功能键。         |
| ENTER_KEY_TYPE_SEARCH      | number   | 是   | 否   | “搜索”功能键。         |
| ENTER_KEY_TYPE_SEND        | number   | 是   | 否   | “发送”功能键。         |
| ENTER_KEY_TYPE_NEXT        | number   | 是   | 否   | “下一个”功能键。       |
| ENTER_KEY_TYPE_DONE        | number   | 是   | 否   | “回车”功能键。         |
| ENTER_KEY_TYPE_PREVIOUS    | number   | 是   | 否   | “前一个”功能键。       |
| PATTERN_NULL               | number   | 是   | 否   | 无特殊性编辑框。       |
| PATTERN_TEXT               | number   | 是   | 否   | 文本编辑框。           |
| PATTERN_NUMBER             | number   | 是   | 否   | 数字编辑框。           |
| PATTERN_PHONE              | number   | 是   | 否   | 电话号码编辑框。       |
| PATTERN_DATETIME           | number   | 是   | 否   | 日期编辑框。           |
| PATTERN_EMAIL              | number   | 是   | 否   | 邮件编辑框。           |
| PATTERN_URI                | number   | 是   | 否   | 超链接编辑框。         |
| PATTERN_PASSWORD           | number   | 是   | 否   | 密码编辑框。           |
| OPTION_ASCII               | number   | 是   | 否   | 允许输入ASCII值。      |
| OPTION_NONE                | number   | 是   | 否   | 不指定编辑框输入属性。 |
| OPTION_AUTO_CAP_CHARACTERS | number   | 是   | 否   | 允许输入字符。         |
| OPTION_AUTO_CAP_SENTENCES  | number   | 是   | 否   | 允许输入句子。         |
| OPTION_AUTO_WORDS          | number   | 是   | 否   | 允许输入单词。         |
| OPTION_MULTI_LINE          | number   | 是   | 否   | 允许输入多行。         |
| OPTION_NO_FULLSCREEN       | number   | 是   | 否   | 半屏样式。             |
| FLAG_SELECTING             | number   | 是   | 否   | 编辑框处于选择状态。   |
| FLAG_SINGLE_LINE           | number   | 是   | 否   | 编辑框为单行。         |
| DISPLAY_MODE_PART          | number   | 是   | 否   | 编辑框显示为半屏。     |
| DISPLAY_MODE_FULL          | number   | 是   | 否   | 编辑框显示为全屏。     |

**使用说明**

```
// 导入模块
import inputmethodengine from '@ohos.inputmethodengine';

// 获取服务端实例
console.log('Get inputMethodEngine');
let inputMethodEngine = inputmethodengine.getInputMethodEngine();

// 获取客户端监听实例
console.log('Get keyboardDelegate');
let keyboardDelegate = inputmethodengine.createKeyboardDelegate();
```

**表 5**   InputMethodEngine开放的主要方法

<table><thead align="left"><tr id="row143351854201012"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p103351154121010"><a name="p103351154121010"></a><a name="p103351154121010"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p1033585416105"><a name="p1033585416105"></a><a name="p1033585416105"></a>描述</p>
</th>
</tr>
</thead>
<tbody>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>on(type: 'inputStart', callback: (kbController: KeyboardController, textInputClient: TextInputClient) => void): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>订阅输入法绑定成功事件，使用callback回调返回输入法操作相关实例。参数个数为2，参数1为napi_string，参数2为napi_function，否则抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>off(type: 'inputStart', callback?: (kbController: KeyboardController, textInputClient: TextInputClient) => void): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>取消订阅输入法绑定成功事件。参数个数不为1或2抛出异常，若为1，参数不为napi_string抛出异常，若为2，参数1不为napi_string，参数2不为napi_function抛出异常。参数若为1，取消此类型所有监听，参数若为2，取消此类型当前监听。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>on(type: 'keyboardShow'|'keyboardHide', callback: () => void): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>订阅输入法事件。参数个数为2，参数1为napi_string，参数2为napi_function，否则抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>off(type: 'keyboardShow'|'keyboardHide', callback?: () => void): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>取消订阅输入法事件。参数个数不为1或2抛出异常，若为1，参数不为napi_string抛出异常，若为2，参数1不为napi_string，参数2不为napi_function抛出异常。参数若为1，取消此类型所有监听，参数若为2，取消此类型当前监听。</p>
</td>
</tr>
</tbody>
</table>

**使用说明**

```
// 导入模块
import inputmethodengine from '@ohos.inputmethodengine';

// 获取服务端实例
console.log('Get inputMethodEngine');
let inputMethodEngine = inputmethodengine.getInputMethodEngine();

// 订阅输入法绑定成功事件
inputMethodEngine.on('inputStart', (kbController, textInputClient) => {
    console.log("inputMethodEngine inputStart, kbController:" + JSON.stringify(kbController));
    console.log("inputMethodEngine inputStart, textInputClient:" + JSON.stringify(textInputClient));
});

// 取消订阅输入法绑定成功事件
inputMethodEngine.off('inputStart', (kbController, textInputClient) => {
    console.log("delete inputStart notification.");
});

// 订阅输入法事件
inputMethodEngine.on('keyboardShow', () => {
    console.log("inputMethodEngine keyboardShow.");
});
inputMethodEngine.on('keyboardHide', () => {
    console.log("inputMethodEngine keyboardHide.");
});

// 取消订阅输入法事件
inputMethodEngine.off('keyboardShow', () => {
    console.log("inputMethodEngine delete keyboardShow notification.");
});
inputMethodEngine.off('keyboardHide', () => {
    console.log("inputMethodEngine delete keyboardHide notification.");
});

```

**表 6**   KeyboardDelegate开放的主要方法

<table><thead align="left"><tr id="row143351854201012"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p103351154121010"><a name="p103351154121010"></a><a name="p103351154121010"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p1033585416105"><a name="p1033585416105"></a><a name="p1033585416105"></a>描述</p>
</th>
</tr>
</thead>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>on(type: 'keyDown'|'keyUp', callback: (event: KeyEvent) => boolean): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>订阅硬键盘事件，使用callback回调返回按键信息。参数个数为2，参数1为napi_string，参数2为napi_function，否则抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>off(type: 'keyDown'|'keyUp', callback?: (event: KeyEvent) => boolean): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>取消订阅硬键盘事件。参数个数不为1或2抛出异常，若为1，参数不为napi_string抛出异常，若为2，参数1不为napi_string，参数2不为napi_function抛出异常。参数若为1，取消此类型所有监听，参数若为2，取消此类型当前监听。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>on(type: 'cursorContextChange', callback: (x: number, y:number, height:number) => void): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>订阅光标变化事件，使用callback回调返回光标信息，使用callback回调返回按键信息。参数个数为2，参数1为napi_string，参数2为napi_function，否则抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>off(type: 'cursorContextChange', callback?: (x: number, y:number, height:number) => void): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>取消订阅光标变化事件。参数个数不为1或2抛出异常，若为1，参数不为napi_string抛出异常，若为2，参数1不为napi_string，参数2不为napi_function抛出异常。参数若为1，取消此类型所有监听，参数若为2，取消此类型当前监听。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>on(type: 'selectionChange', callback: (oldBegin: number, oldEnd: number, newBegin: number, newEnd: number) => void): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>订阅文本选择变化事件，使用callback回调返回文本选择信息，使用callback回调返回按键信息。参数个数为2，参数1为napi_string，参数2为napi_function，否则抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>off(type: 'selectionChange', callback?: (oldBegin: number, oldEnd: number, newBegin: number, newEnd: number) => void): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>取消订阅文本选择变化事件。参数个数不为1或2抛出异常，若为1，参数不为napi_string抛出异常，若为2，参数1不为napi_string，参数2不为napi_function抛出异常。参数若为1，取消此类型所有监听，参数若为2，取消此类型当前监听。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>on(type: 'textChange', callback: (text: string) => void): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>订阅文本变化事件，使用callback回调返回当前文本内容，使用callback回调返回按键信息。参数个数为2，参数1为napi_string，参数2为napi_function，否则抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>off(type: 'textChange', callback?: (text: string) => void): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>取消订阅文本变化事件。参数个数不为1或2抛出异常，若为1，参数不为napi_string抛出异常，若为2，参数1不为napi_string，参数2不为napi_function抛出异常。参数若为1，取消此类型所有监听，参数若为2，取消此类型当前监听。</p>
</td>
</tr>
</tbody>
</table>

**使用说明**

```
// 导入模块
import inputmethodengine from '@ohos.inputmethodengine';

// 获取客户端监听实例
console.log('Get keyboardDelegate');
let keyboardDelegate = inputmethodengine.createKeyboardDelegate();

// 订阅硬键盘事件
keyboardDelegate.on('keyUp', (keyEvent) => {
    console.info("inputMethodEngine keyCode.(keyUp):" + JSON.stringify(keyEvent.keyCode));
    console.info("inputMethodEngine keyAction.(keyUp):" + JSON.stringify(keyEvent.keyAction));
    return true;
});
keyboardDelegate.on('keyDown', (keyEvent) => {
    console.info("inputMethodEngine keyCode.(keyDown):" + JSON.stringify(keyEvent.keyCode));
    console.info("inputMethodEngine keyAction.(keyDown):" + JSON.stringify(keyEvent.keyAction));
    return true;
});

// 取消订阅硬键盘事件
keyboardDelegate.off('keyUp', (keyEvent) => {
    console.log("delete keyUp notification.");
    return true;
});
keyboardDelegate.off('keyDown', (keyEvent) => {
    console.log("delete keyDown notification.");
    return true;
});

// 订阅光标变化事件
keyboardDelegate.on('cursorContextChange', (x, y, height) => {
    console.log("inputMethodEngine cursorContextChange x:" + x);
    console.log("inputMethodEngine cursorContextChange y:" + y);
    console.log("inputMethodEngine cursorContextChange height:" + height);
});

// 取消订阅光标变化事件
keyboardDelegate.off('cursorContextChange', (x, y, height) => {
    console.log("delete cursorContextChange notification.");
});

// 订阅文本选择变化事件
keyboardDelegate.on('selectionChange', (oldBegin, oldEnd, newBegin, newEnd) => {
    console.log("inputMethodEngine beforeEach selectionChange oldBegin:" + oldBegin);
    console.log("inputMethodEngine beforeEach selectionChange oldEnd:" + oldEnd);
    console.log("inputMethodEngine beforeEach selectionChange newBegin:" + newBegin);
    console.log("inputMethodEngine beforeEach selectionChange newEnd:" + newEnd);
});

// 取消订阅文本选择变化事件
keyboardDelegate.off('selectionChange', (oldBegin, oldEnd, newBegin, newEnd) => {
  console.log("delete selectionChange notification.");
});

// 订阅文本变化事件
keyboardDelegate.on('textChange', (text) => {
    console.log("inputMethodEngine textChange. text:" + text);
});

// 取消订阅文本变化事件
keyboardDelegate.off('textChange', (text) => {
    console.log("delete textChange notification. text:" + text);
});
```

**表 7**   KeyboardController开放的主要方法

<table><thead align="left"><tr id="row143351854201012"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p103351154121010"><a name="p103351154121010"></a><a name="p103351154121010"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p1033585416105"><a name="p1033585416105"></a><a name="p1033585416105"></a>描述</p>
</th>
</tr>
</thead>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>hideKeyboard(callback: AsyncCallback&lt;void&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>隐藏输入法，使用callback形式返回结果。参数个数为1，否则抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>hideKeyboard(): Promise&lt;void&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>隐藏输入法，使用peomise形式返回结果。参数个数为0，否则抛出异常。</p>
</td>
</tr>
</tbody>
</table>

**使用说明**

```
// 导入模块
import inputmethodengine from '@ohos.inputmethodengine';

var kbCtrl = null;
console.log('Get inputMethodEngine');
let inputMethodEngine = inputmethodengine.getInputMethodEngine();
inputMethodEngine.on('inputStart', (kbController, textInputClient) => {
    console.log("inputMethodEngine beforeEach inputStart:" + JSON.stringify(kbController));
    kbCtrl = kbController;
});

// 隐藏输入法callback
kbCtrl.hideKeyboard((err) => {
    if (err) {
        console.error("hideKeyboard callback result---err: " + err.msg);
        return;
    }
    console.log("hideKeyboard callback.");
});

// 隐藏输入法promise
await kbCtrl.hideKeyboard().then(() => {
    console.info("hideKeyboard promise.");
}).catch((err) => {
    console.info("hideKeyboard promise err: " + err.msg);
});
```



**表 8**   TextInputClient开放的主要方法

<table><thead align="left"><tr id="row143351854201012"><th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.1"><p id="p103351154121010"><a name="p103351154121010"></a><a name="p103351154121010"></a>接口名</p>
</th>
<th class="cellrowborder" valign="top" width="50%" id="mcps1.2.3.1.2"><p id="p1033585416105"><a name="p1033585416105"></a><a name="p1033585416105"></a>描述</p>
</th>
</tr>
</thead>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>getForward(length:number, callback: AsyncCallback&lt;string&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>获取光标前固定长度的文本，使用callback形式返回结果。参数个数为2，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>getForward(length:number): Promise&lt;string&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>获取光标前固定长度的文本,使用promise形式返回结果。参数个数为1，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>getBackward(length:number, callback: AsyncCallback&lt;string&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>获取光标后固定长度的文本，使用callback形式返回结果。参数个数为2，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>getBackward(length:number): Promise&lt;string&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>获取光标后固定长度的文本，使用promise形式返回结果。参数个数为1，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>deleteForward(length:number, callback: AsyncCallback&lt;boolean&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>删除光标前固定长度的文本，使用callback形式返回结果。参数个数为2，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>deleteForward(length:number): Promise&lt;boolean&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>删除光标前固定长度的文本，使用promise形式返回结果。参数个数为1，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>deleteBackward(length:number, callback: AsyncCallback&lt;boolean&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>删除光标后固定长度的文本，使用callback形式返回结果。参数个数为2，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>deleteBackward(length:number): Promise&lt;boolean&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>删除光标后固定长度的文本，使用promise形式返回结果。参数个数为1，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>sendKeyFunction(action:number, callback: AsyncCallback&lt;boolean&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>发送功能键，使用callback形式返回结果。参数个数为2，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>sendKeyFunction(action:number): Promise&lt;boolean&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>发送功能键，使用promise形式返回结果。参数个数为1，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>insertText(text:string, callback: AsyncCallback&lt;boolean&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>插入文本，使用callback形式返回结果。参数个数为2，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>insertText(text:string): Promise&lt;boolean&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>插入文本，使用promise形式返回结果。参数个数为1，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>getEditorAttribute(callback: AsyncCallback&lt;EditorAttribute&gt;): void;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>获取编辑框属性值，使用callback形式返回结果。参数个数为1，否侧抛出异常。</p>
</td>
</tr>
<tr id="row204321219393"><td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.1 "><p id="p1893413268144"><a name="p1893413268144"></a><a name="p1893413268144"></a>getEditorAttribute(): Promise&lt;EditorAttribute&gt;;</p>
</td>
<td class="cellrowborder" valign="top" width="50%" headers="mcps1.2.3.1.2 "><p id="p18761104812149"><a name="p18761104812149"></a><a name="p18761104812149"></a>获取编辑框属性值，使用promise形式返回结果。参数个数为0，否侧抛出异常。</p>
</td>
</tr>
</tbody>
</table>

**使用说明**

```
// 导入模块
import inputmethodengine from '@ohos.inputmethodengine';

var client = null;
var length = 1;
var keyFunction = 0;
console.log('Get inputMethodEngine');
let inputMethodEngine = inputmethodengine.getInputMethodEngine();
inputMethodEngine.on('inputStart', (kbController, textInputClient) => {
    console.log("inputMethodEngine inputStart:" + JSON.stringify(textInputClient));
    client = textInputClient;
});

// 获取光标前固定长度的文本callback
client.getForward(length, (err, text) => {
    if (err) {
        console.error("getForward callback result---err: " + err.msg);
        return;
    }
    console.log("getForward callback result---text: " + text);
});

// 获取光标前固定长度的文本promise
await client.getForward(length).then((text) => {
    console.info("getForward promise result---res: " + text);
}).catch((err) => {
    console.error("getForward promise err: " + err.msg);
});

// 获取光标后固定长度的文本callback
client.getBackward(length, (err, text) => {
    if (err) {
        console.error("getBackward callback result---err: " + err.msg);
        return;
    }
    console.log("getBackward callback result---text: " + text);
});

// 获取光标后固定长度的文本promise
await client.getBackward(length).then((text) => {
    console.info("getBackward promise result---res: " + text);
}).catch((err) => {
    console.error("getBackward promise err: " + err.msg);
});

// 删除光标前固定长度的文本callback
client.deleteForward(length, (err, result) => {
    if (err) {
        console.error('deleteForward callback result---err: ' + err.msg);
        return;
    }
    if (result) {
        console.info("Success to deleteForward.(callback) ");
    } else {
        console.error("Failed to deleteForward.(callback) ");
    }
});

// 删除光标前固定长度的文本promise
await client.deleteForward(length).then((result) => {
    if (result) {
        console.info("Success to deleteForward.(promise) ");
    } else {
        console.error("Failed to deleteForward.(promise) ");
    }
}).catch((err) => {
    console.error("deleteForward promise err: " + err.msg);
});

// 删除光标前固定长度的文本callback
client.deleteBackward(length, (err, result) => {
    if (err) {
        console.error("deleteBackward callback result---err: " + err.msg);
        return;
    }
    if (result) {
        console.info("Success to deleteBackward.(callback) ");
    } else {
        console.error("Failed to deleteBackward.(callback) ");
    }
});

// 删除光标前固定长度的文本promise
await client.deleteBackward(length).then((result) => {
    if (result) {
        console.info("Success to deleteBackward.(promise) ");
    } else {
        console.error("Failed to deleteBackward.(promise) ");
    }
}).catch((err) => {
    console.error("deleteBackward promise err: " + err.msg);
});

// 发送功能键callback
client.sendKeyFunction(keyFunction, (err, result) => {
    if (err) {
        console.error("sendKeyFunction callback result---err: " + err.msg);
        return;
    }
    if (result) {
        console.info("Success to sendKeyFunction.(callback) ");
    } else {
        console.error("Failed to sendKeyFunction.(callback) ");
    }
});

// 发送功能键promise
await client.sendKeyFunction(keyFunction).then((result) => {
    if (result) {
        console.info("Success to sendKeyFunction.(promise) ");
    } else {
        console.error("Failed to sendKeyFunction.(promise) ");
    }
}).catch((err) => {
    console.error("sendKeyFunction promise err:" + err.msg);
});

// 插入文本callback
client.insertText('test', (err, result) => {
    if (err) {
        console.error("insertText callback result---err: " + err.msg);
        return;
    }
    if (result) {
        console.info("Success to insertText.(callback) ");
    } else {
        console.error("Failed to insertText.(callback) ");
    }
});

// 插入文本promise
await client.insertText('test').then((result) => {
    if (result) {
        console.info("Success to insertText.(promise) ");
    } else {
        console.error("Failed to insertText.(promise) ");
    }
}).catch((err) => {
    console.error("insertText promise err: " + err.msg);
});

// 获取编辑框属性值callback
client.getEditorAttribute((err, editorAttribute) => {
    if (err) {
        console.error("getEditorAttribute callback result---err: " + err.msg);
        return;
    }
    console.log("editorAttribute.inputPattern(callback): " + JSON.stringify(editorAttribute.inputPattern));
    console.log("editorAttribute.enterKeyType(callback): " + JSON.stringify(editorAttribute.enterKeyType));
});

// 获取编辑框属性值promise
await client.getEditorAttribute().then((editorAttribute) => {
    console.info("editorAttribute.inputPattern(promise): " + JSON.stringify(editorAttribute.inputPattern));
    console.info("editorAttribute.enterKeyType(promise): " + JSON.stringify(editorAttribute.enterKeyType));
}).catch((err) => {
    console.error("getEditorAttribute promise err: " + err.msg);
});
```



**EditorAttribute**

**编辑框属性值**

| 名称         | 参数类型 | 可读 | 可写 | 说明               |
| ------------ | -------- | ---- | ---- | ------------------ |
| enterKeyType | number   | 是   | 否   | 编辑框的功能属性。 |
| inputPattern | number   | 是   | 否   | 编辑框的文本属性。 |

**KeyEvent**

**按键属性值**

| 名称      | 参数类型 | 可读 | 可写 | 说明         |
| --------- | -------- | ---- | ---- | ------------ |
| keyCode   | number   | 是   | 否   | 按键的键值。 |
| keyAction | number   | 是   | 否   | 按键的状态。 |



#### 框架主要支持功能

1.在编辑属性的控件中进行点击操作，即可通过输入法框架调起默认输入法应用

2.通过输入法应用可以进行打字，并上屏输入字符到应用客户端

#### 本框架编译调试方法

1.   编译命令

./build.sh --product-name (填写具体的产品名，如：Hi3516DV300) --build-target imf

2.  推送so文件

将工程目录下out\ohos-arm-release\inputmethod\imf 下的libinputmethod_client.z.so libinputmethod_ability.z.so 
libinputmethod_service.z.so libinputmethod_para.z.so推送到system/lib，将libinputmethodengine.z.so libinputmethod.z.so 推送到system/lib/module下，并确保六个so至少为可读状态。

3.  重启设备

#### 参与贡献

1.  Fork 本仓库
2.  提交代码
3.  新建 Pull Request
4.  commit完成即可

