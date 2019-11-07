package foo

import scala.swing._

class HelloWorld extends MainFrame {
  def createMenuBar(): MenuBar = {
    new MenuBar() {
      contents += createMenu("File")
      contents += createMenu("Edit")
      contents += createMenu("Help")
    }
  }

  def createMenu(title: String): Menu = {
    new Menu(title) {
      contents += new MenuItem(s"Menu item #1 in $title")
    }
  }
}

object HelloWorld extends SimpleSwingApplication {
  lazy val top = new HelloWorld() {
    title = "HelloWorld"
    menuBar = createMenuBar()
    contents = new GridPanel(1, 1) {
      contents += new Label("This is a label.") {
         preferredSize = new Dimension(640, 480)
      }  
    }
    size = new Dimension(640, 480)
  }
}

