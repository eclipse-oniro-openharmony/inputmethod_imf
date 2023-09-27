@Entry
@Component
struct Index {
  @State message: string = 'Hello World'
  data: string = ''
  build() {
    Row() {
      Column() {
        TextInput({ placeholder: "输入内容", text: this.data })
          .onChange((data) => {
            this.data = data
          }).width('100%').height('100%')
      }
      .width('100%')
    }
    .height('100%')
  }
}