#include "ImageWidget.h"
#include <cstdlib>

ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent) {
  setMouseTracking(true);
  isDragging_ = false;
  isSelecting_ = false;
  isSelectionEnabled_ = false;
  selectionType_ = SelectionType::Rectangle;
  selectionColor_ = Qt::cyan;
}

void ImageWidget::setImageSize(int width, int height, QImage::Format format){
  if(width_ != width || height_ != height) {
    width_ = width;
    height_ = height;
    img_ = std::make_unique<QImage>(width, height, format);
    img_->fill(Qt::black);
  }
}

void ImageWidget::setImageSource(QImage* image, int width, int height) {
  if(image == nullptr) {
    setImageSize(1280, 960);
    img_->fill(Qt::black);
  } else {
    setImageSize(width, height);
    *img_ = *image;
  }
  update();
}

void ImageWidget::paintEvent(QPaintEvent *event) {
  if(img_ == nullptr) return;
  QPainter painter;
  painter.begin(this);

  if(zoomstack_.size() == 0)
    painter.drawImage(event->rect(), *img_);
  else{
    QRect viewbox = getViewBox();
    painter.drawImage(QRect(0,0,width(),height()), *img_, viewbox);
  }
  painter.end();
  if(isSelecting_)
    drawSelection();
  if(isSelectionEnabled_)
    drawStoredSelections();
}

SelectionMode ImageWidget::getMode(){
  int mods = QApplication::keyboardModifiers();
  if(mods == Qt::NoModifier)
    return None;
  else if (mods == (Qt::ControlModifier | Qt::AltModifier))
    return Zoom;
  else if (isSelectionEnabled_ && mods == Qt::AltModifier )
    return Subtract;
  else if (isSelectionEnabled_ && mods == Qt::ControlModifier )
    return Add;
  return None;
}

SelectionType ImageWidget::getType() {
  if(getMode() == Zoom)
    return SelectionType::Rectangle;
  else
    return selectionType_;
}

/** Mouse Events **/

void ImageWidget::mousePressEvent(QMouseEvent *event) {
  currentX_ = startX_ = event->x();
  currentY_ = startY_ = event->y();
  if(getMode() == None) {
    if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton) return;
    clearSelection();
    QPoint point = mapPoint(QPoint(event->x(),event->y()));
    emit clicked(point.x(), point.y(), (int) event->button());
  }
  else if (getMode() == Zoom){
    if(event->button() == Qt::RightButton){
      zoomOut();
    }
  }
  else if (getMode() == Add && (getType() == SelectionType::Polygon || getType() == SelectionType::Manual)) {
    if (event->button() == Qt::LeftButton && !isSelecting_)
      beginSelection();
    else if (event->button() == Qt::LeftButton && isSelecting_)
      selectionVertices_.push_back(mapPoint(QPoint(startX_, startY_)));
    else if(event->button() == Qt::RightButton)
      endSelection();
  }
  else if (getMode() == Add && getType() == SelectionType::Ellipse){
    if (event->button() == Qt::LeftButton && !isSelecting_)
      beginSelection();
    else if (event->button() == Qt::LeftButton && isSelecting_ && selectionVertices_.size() < 3)
      selectionVertices_.push_back(mapPoint(QPoint(startX_, startY_)));
    if(selectionVertices_.size() == 3)
      endSelection();
  }
  repaint();
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event) {
  QPoint point = mapPoint(QPoint(event->x(),event->y()));
  emit mouseXY(point.x(),point.y());
  if(QApplication::mouseButtons() == Qt::LeftButton && (getMode() == Zoom || getMode() == Add) && getType() == SelectionType::Rectangle) {
    isDragging_ = true;
    beginSelection();
    updateSelection(event->x(), event->y());
  }
  else if (getMode() == Add && getType() == SelectionType::Manual) {
    isDragging_ = true;
    QPoint p(event->x(), event->y());
    p = mapPoint(p);
    if(QApplication::mouseButtons() == Qt::LeftButton)
      selectionVertices_.push_back(p);
    updateSelection(event->x(), event->y());
  }
  else if(getMode() == Add && (getType() == SelectionType::Polygon || getType() == SelectionType::Ellipse))
    updateSelection(event->x(), event->y());
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *) {
  if(isDragging_){
    if(getType() != SelectionType::Manual)
      endSelection();
    isDragging_ = false;
    repaint();
    return;
  }
}

/** Selection **/

void ImageWidget::setCurrentSelections(std::vector<Selection*> selections) {
  selections_ = selections;
  repaint();
}

void ImageWidget::beginSelection(){
  isSelecting_ = true;
  switch(getType()) {
    case SelectionType::Ellipse:
    case SelectionType::Polygon:
    case SelectionType::Manual:
      selectionVertices_.clear();
      selectionVertices_.push_back(mapPoint(QPoint(startX_, startY_)));
      break;
    default:
      break;
  }
}

void ImageWidget::updateSelection(int x, int y){
  currentX_ = x;
  currentY_ = y;
  repaint();
}

void ImageWidget::endSelection(){
  isSelecting_ = false;
  if(getMode() == Zoom){
    zoomIn(startX_, startY_, currentX_, currentY_);
    repaint();
  }
  if(getMode() == Add && getType() == SelectionType::Rectangle){
    QPoint topleft(startX_, startY_), bottomright(currentX_, currentY_);
    topleft = mapPoint(topleft);
    bottomright = mapPoint(bottomright);
    UTRectangle* rect = new UTRectangle(Point(topleft.x(), topleft.y()), Point(bottomright.x(), bottomright.y()));
    emit selected(rect);
  }
  if(getMode() == Add && (getType() == SelectionType::Polygon || getType() == SelectionType::Manual)){
    std::vector<Point> vertices;
    for(uint16_t i = 0; i < selectionVertices_.size(); i++)
      vertices.push_back(Point(selectionVertices_[i].x(), selectionVertices_[i].y()));
    
    UTPolygon* poly = new UTPolygon(vertices);
    emit selected(poly);
  }
  if(getMode() == Add && getType() == SelectionType::Ellipse){
    std::vector<Point> vertices;
    for(uint16_t i = 0; i < selectionVertices_.size(); i++)
      vertices.push_back(Point(selectionVertices_[i].x(), selectionVertices_[i].y()));
    UTEllipse* ellipse = new UTEllipse(vertices);
    ellipse->fixpoints();
    emit selected(ellipse);
  }
}

void ImageWidget::clearSelection(){
  isSelecting_ = false;
  isDragging_ = false;
  selectionVertices_.clear();
}

/** Drawing **/

void ImageWidget::drawSelection(){
  switch(getType()){
    case SelectionType::Rectangle:
      drawSelectionRectangle(getMode() != Zoom);
      break;
    case SelectionType::Manual:
    case SelectionType::Polygon:
      drawSelectionPolygon();
      break;
    case SelectionType::Ellipse:
      drawSelectionEllipse();
      break;
    default:
      break;
  }
}

void ImageWidget::drawSelectionRectangle(bool map){
  QPoint topleft(startX_, startY_), bottomright(currentX_, currentY_);
  if(map) {
    topleft = mapPoint(topleft);
    bottomright = mapPoint(bottomright);
  }
  UTRectangle rect(Point(topleft.x(), topleft.y()), Point(bottomright.x(), bottomright.y()));
  drawRectangle(&rect, map);
}

void ImageWidget::drawSelectionEllipse(){
  std::vector<Point> vertices;
  for(uint16_t i = 0; i < selectionVertices_.size(); i++)
    vertices.push_back(Point(selectionVertices_[i].x(), selectionVertices_[i].y()));
  UTEllipse ellipse(vertices);
  if(isSelecting_) {
    QPoint qp(currentX_, currentY_);
    qp = mapPoint(qp);
    Point p(qp.x(), qp.y());
    ellipse.addVertex(p);
  }
  ellipse.fixpoints();
  drawEllipse(&ellipse);
}

void ImageWidget::drawSelectionPolygon(){
  std::vector<Point> vertices;
  for(uint16_t i = 0; i < selectionVertices_.size(); i++)
    vertices.push_back(Point(selectionVertices_[i].x(), selectionVertices_[i].y()));
  UTPolygon poly(vertices);
  if(isSelecting_) {
    QPoint qp(currentX_, currentY_);
    qp = mapPoint(qp);
    Point p(qp.x(), qp.y());
    poly.addVertex(p);
  }
  drawPolygon(&poly);
}

void ImageWidget::drawStoredSelections() {
  for(unsigned int i=0; i<selections_.size(); i++) {
    switch(selections_[i]->getSelectionType()){
      case SelectionType::Rectangle:
        drawRectangle((UTRectangle*)selections_[i], true);
        break;
      case SelectionType::Ellipse:
        drawEllipse((UTEllipse*)selections_[i]);
        break;
      case SelectionType::Polygon:
        drawPolygon((UTPolygon*)selections_[i]);
        break;
      default:
        break;
    }
  }
}

void ImageWidget::drawRectangle(UTRectangle* rectangle, bool map){
  QPainter painter;
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(selectionColor_);
  painter.setOpacity(.7);
  QRect rect;
  QPoint topleft = QPoint(rectangle->getTopLeft().x, rectangle->getTopLeft().y);
  QPoint bottomright = QPoint(rectangle->getBottomRight().x, rectangle->getBottomRight().y);
  if(map)
    rect = QRect(unMapPoint(topleft), unMapPoint(bottomright));
  else
    rect = QRect(topleft, bottomright);
  painter.drawRect(rect);
  painter.setBrush(selectionColor_);
  painter.setOpacity(.2);
  painter.drawRect(rect);
  painter.end();
}

void ImageWidget::drawEllipse(UTEllipse* ellipse){
  QPainter painter;
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(selectionColor_);
  QPoint topleft(ellipse->x(), ellipse->y());
  QPoint bottomright(ellipse->x() + ellipse->width(), ellipse->y() + ellipse->height());
  QRect rect(unMapPoint(topleft), unMapPoint(bottomright));
  painter.drawEllipse(rect);
  painter.setBrush(selectionColor_);
  painter.setOpacity(.2);
  painter.drawEllipse(rect);
  painter.end();
}

void ImageWidget::drawPolygon(UTPolygon* poly) {
  QPainter painter;
  painter.begin(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(selectionColor_);
  std::vector<Point> vertices = poly->getVertices();
  int vcount = vertices.size();
  QPoint* points = new QPoint[vcount];
  for(int j=0; j<vcount; j++){
    Point p = vertices[j];
    QPoint qp = QPoint(p.x,p.y);
    points[j] = unMapPoint(qp);
  }
  painter.setOpacity(.7);
  painter.drawPolygon(points, vcount);
  painter.setBrush(selectionColor_);
  painter.setOpacity(.2);
  painter.drawPolygon(points,vcount);
  painter.end();
  delete [] points;
}

/** Zooming **/

void ImageWidget::zoomIn(int startx, int starty, int endx, int endy){
  QRect zoom(std::min(startx,endx), std::min(starty,endy), std::abs(startx - endx), std::abs(starty - endy));
  zoomstack_.push_back(zoom);
}

void ImageWidget::zoomOut(){
  if(zoomstack_.size() > 0)
    zoomstack_.pop_back();
}

QRect ImageWidget::getViewBox(){
  QRect viewbox(0,0,width_,height_);
  for(unsigned int i=0; i<zoomstack_.size();i++) {
    QRect zoom = zoomstack_[i];
    float currentWidth = viewbox.width(), currentHeight = viewbox.height();
    viewbox.setLeft(viewbox.left() + zoom.left() * currentWidth / width());
    viewbox.setRight( viewbox.right() - (width() - zoom.right()) * currentWidth / width() );
    viewbox.setTop(viewbox.top() + zoom.top() * currentHeight / height());
    viewbox.setBottom(viewbox.bottom() - (height() - zoom.bottom()) * currentHeight / height());
  }
  return viewbox;
}

// Maps a point in the current frame to the global coordinates
QPoint ImageWidget::mapPoint(QPoint point) {
  QRect viewbox = getViewBox();
  float xpct = (float)point.x() / width(), ypct = (float)point.y() / height();
  point.setX(xpct * viewbox.width() + viewbox.left());
  point.setY(ypct * viewbox.height() + viewbox.top());
  return point;
}

// Maps a point in the global frame to the zoomed coordinates
QPoint ImageWidget::unMapPoint(QPoint point) {
  QRect viewbox = getViewBox();
  float xpct = ((float)point.x() - viewbox.left()) / viewbox.width(), ypct = ((float)point.y() - viewbox.top()) / viewbox.height();
  point.setX(xpct * width());
  point.setY(ypct * height());
  return point;
}

void ImageWidget::setSelectionEnabled(bool value){
  isSelectionEnabled_ = value;
  repaint();
}

void ImageWidget::selectionTypeChanged(SelectionType type) {
  selectionType_ = type;
}

bool ImageWidget::getSelectionEnabled() {
  return isSelectionEnabled_;
}
