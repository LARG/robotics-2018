#include <VisionWindow.h>

void VisionWindow::bottomNewTable() {
  newTable(Camera::BOTTOM);
}

void VisionWindow::bottomSaveTableAs() {
  saveTableAs(Camera::BOTTOM);
}

void VisionWindow::bottomSaveTable() {
  saveTable(Camera::BOTTOM);
}

void VisionWindow::bottomOpenTable() {
  openTable(Camera::BOTTOM);
}

void VisionWindow::topNewTable() {
  newTable(Camera::TOP);
}

void VisionWindow::topSaveTableAs() {
  saveTableAs(Camera::TOP);
}

void VisionWindow::topSaveTable() {
  saveTable(Camera::TOP);
}

void VisionWindow::topOpenTable() {
  openTable(Camera::TOP);
}

void VisionWindow::newTable(Camera::Type camera) {
  unsigned char* colorTable;
  if (camera == Camera::TOP) {
    colorTable = core_->vision_->topColorTable;
    core_->vision_->topColorTableName = "none";
  } else {
    colorTable = core_->vision_->bottomColorTable;
    core_->vision_->bottomColorTableName = "none";
  }
  memset(colorTable,c_UNDEFINED,LUT_SIZE);
  emit colorTableLoaded();
}

void VisionWindow::openTable(Camera::Type camera) {
  QString file = QFileDialog::getOpenFileName(this, 
    tr("Open Color Table"),
    QString(getenv("NAO_HOME")) + "/data/current",
    tr("UT Color Tables (*.col)"),
    0, QFileDialog::DontUseNativeDialog
  );
  if (file.isEmpty())
    return;

  core_->vision_->loadColorTable(camera, file.toStdString(), true);

  emit colorTableLoaded();
}

void VisionWindow::writeTable(Camera::Type camera, std::string fileName) {

  unsigned char *colorTable;
  std::string displayMessage;
  if (camera == Camera::TOP) {
    colorTable = core_->vision_->topColorTable;
    displayMessage += "Top Table";
  } else {
    colorTable = core_->vision_->bottomColorTable;
    displayMessage += "Bottom Table";
  }

  FILE* f = fopen(fileName.c_str(), "wb");
  fwrite(colorTable, LUT_SIZE, 1, f);
  fclose(f);

  displayMessage = "Wrote " + displayMessage + "to file: " + std::string(fileName);
  std::cout << displayMessage << std::endl;

}

void VisionWindow::saveTable(Camera::Type camera) {

  std::string colorTableName;
  if (camera == Camera::TOP) {
    colorTableName = core_->vision_->topColorTableName;
  } else {
    colorTableName = core_->vision_->bottomColorTableName;
  }

  if ((colorTableName.c_str(), "none") == 0) {
    saveTableAs(camera);
    return;
  }

  writeTable(camera, colorTableName);
}

void VisionWindow::saveTableAs(Camera::Type camera) {
  QString file = QFileDialog::getSaveFileName(this, 
    tr("Save Color Table"),
    QString(getenv("NAO_HOME")) + "/data",
    tr("UT Color Tables (*.col)"),
    0, QFileDialog::DontUseNativeDialog
  );
  if (file.isEmpty()) return;
  std::string fileName = file.toStdString();

  writeTable(camera, fileName);

  if (camera == Camera::TOP) {
    core_->vision_->topColorTableName = file.toStdString();
  } else {
    core_->vision_->bottomColorTableName = file.toStdString();
  }

}
