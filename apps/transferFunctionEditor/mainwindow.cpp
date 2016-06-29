/* -*- mode: c++ -*-
 *
 * apps/transferFunctionEditor/mainwindow.cpp --
 *
 * Initial software
 * Authors: Izzatbek Mukhanov
 * Copyright © Inria
 *
 * http://libpointing.org/
 *
 * This software may be used and distributed according to the terms of
 * the GNU General Public License version 2 or any later version.
 *
 */

// TODO Plug-in plug-out of input devices

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <pointing/utils/FileUtils.h>
#include <pointing/utils/ConfigDict.h>
#include <pointing/input/PointingDeviceManager.h>
#include <fstream>

using namespace std;
using namespace pointing;

#define ipm 0.0254f

pointing::TransferFunction *MainWindow::func = NULL;

void MainWindow::recomputeFunc(TF &tf)
{
  tf.points = vector<double>(128, 0.);
  TransferFunction *f = TransferFunction::create(tf.uri, input, output);
  TimeStamp::inttime now = TimeStamp::createAsInt();
  double updateFreq = input->getUpdateFrequency();

  int deltaMs = 1000. / updateFreq;

  for (unsigned i = 1; i < tf.points.size(); i++)
  {
    f->clearState();
    double x, y;
    f->applyd(i, 0, &x, &y, now + i * deltaMs * TimeStamp::one_millisecond);
    double speedInput = i / input->getResolution() * ipm * input->getUpdateFrequency();
    double speedOutput = x / output->getResolution() * ipm * input->getUpdateFrequency();
    double gain = speedOutput / speedInput;
    tf.points[i] = gain;

    if (gain > maxGain)
    {
      maxGain = gain;
    }
  }
  delete f;
}

void MainWindow::recomputeAllFuncs()
{
  maxGain = 0;
  for (TF &tf : allTfs)
    recomputeFunc(tf);
}

list<TF> MainWindow::getDefaultTfs()
{
  list<TF> result;
  string dir = moduleHeadersPath() + "\\pointing-echomouse\\";
  result.push_back(TF("Windows", "windows:?", TFTypes::Echomouse, "windows\\epp"));
  result.push_back(TF("Windows without EPP", "windows:?epp=false", TFTypes::Echomouse, "windows\\no-epp"));
  result.push_back(TF("Sigmoid", "sigmoid:?", TFTypes::Default));
  result.push_back(TF("Linux", "xorg:?", TFTypes::Default));
  result.push_back(TF("Darwin 10", "", TFTypes::Echomouse, "darwin-10"));
  result.push_back(TF("Darwin 14", "", TFTypes::Echomouse, "darwin-14"));
  result.push_back(TF("Darwin 15", "osx:?", TFTypes::Echomouse, "darwin-15"));
  result.push_back(TF("Logitech High Acceleration", "", TFTypes::Echomouse, "logitech\\acceleration-high"));
  result.push_back(TF("Logitech Medium Acceleration", "", TFTypes::Echomouse, "logitech\\acceleration-medium"));
  result.push_back(TF("Logitech Low Acceleration", "", TFTypes::Echomouse, "logitech\\acceleration-low"));
  result.push_back(TF("Logitech No Acceleration", "", TFTypes::Echomouse, "logitech\\acceleration-none"));
  for (TF & tf : result)
  {
    if (!tf.uri.length())
    {
      tf.uri = "interp:" + dir + tf.path;
    }
  }
  return result;
}

string ws2s(const wstring& string_to_convert)
{
  wstring wide(string_to_convert);
  return string(wide.begin(), wide.end());
}

list<string> getDirectoryFunctions(wstring path, wstring prefix)
{
  list<string> result;
  wstring allpath = path + wstring(L"\\*");
  WIN32_FIND_DATA ffd;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  hFind = FindFirstFile(allpath.c_str(), &ffd);
  if (INVALID_HANDLE_VALUE == hFind)
  {
    cerr << "Invalid Handle Value" << endl;
    return result;
  }

  // List all the files in the directory with some info about them.
  do
  {
    wstring dirPath = path + wstring(L"\\") + wstring(ffd.cFileName);
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      wstring configDictPath = dirPath + wstring(L"\\config.dict");
      // This directory contains config.dict
      if (ifstream(ws2s(configDictPath).c_str()))
      {
        result.push_back(ws2s(prefix + wstring(ffd.cFileName)));
      }
      else // Look subdirectories
      {
        wstring subfolder(ffd.cFileName);
        if (subfolder != wstring(L".") && subfolder != wstring(L".."))
        {
          list<string> subResult = getDirectoryFunctions(dirPath, prefix + subfolder + wstring(L"\\"));
          result.insert(result.end(), subResult.begin(), subResult.end());
        }
      }
    }
  }
  while (FindNextFile(hFind, &ffd) != 0);
  FindClose(hFind);
  return result;
}

bool MainWindow::isInLibpointing(string path)
{
  for (TF tf : allTfs)
  {
    if (path == tf.path)
      return true;
  }
  return false;
}

void MainWindow::updateList()
{
  ui->listWidget->clear();
  string outputDir = moduleHeadersPath() + "\\pointing-echomouse";
  wstring woutputDir = wstring(outputDir.begin(), outputDir.end());
  list<string> all = getDirectoryFunctions(woutputDir, L"");
  for (string &itemPath : all)
  {
    if (!isInLibpointing(itemPath))
    {
      string uri = "interp:" + outputDir + "\\" + itemPath;
      allTfs.push_back(TF(itemPath, uri, TFTypes::Custom, itemPath));
    }
  }
  recomputeAllFuncs();

  int i = 0;
  for (TF & tf : allTfs)
  {
    ui->customPlot->addGraph();
    //ui->customPlot->graph(i)->setName(QString::fromStdString(tf.name));

    QListWidgetItem *item = new QListWidgetItem(QString::fromStdString(tf.name), ui->listWidget);
    item->setFlags(item->flags() | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Unchecked);
    tf.plotIndex = i++;
  }
}

void MainWindow::setupInput()
{
  PointingDeviceManager *pdm = PointingDeviceManager::get();
  for (PointingDescriptorIterator it = pdm->begin(); it != pdm->end(); it++)
  {
    PointingDeviceDescriptor desc = *it;
    QString text = QString::fromStdString(desc.vendor + " - " + desc.product);
    ui->comboBoxInput->addItem(text, QString::fromStdString(desc.devURI.asString()));
    PointingDevice *dev = PointingDevice::create(desc.devURI.asString());
    inputs.push_back(dev);
    dev->setPointingCallback(pointingCallback, nullptr);
  }

  if (inputs.size())
  {
    input = inputs.front();//PointingDevice::create("winhid:/65611?debugLevel=1&cpi=400&hz=1000");
    updateInput();
  }
  else
  {
    QMessageBox msgBox;
    msgBox.setText("No Pointing device was found");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
  }
}

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  setupInput();

  output = DisplayDevice::create("any:?");
  curName = "Windows";

  SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);

  allTfs = getDefaultTfs();
  updateList();

  TF &tf = allTfs.front();
  tf.shown = true;
  changeShowPlot(tf);
  ui->listWidget->item(0)->setCheckState(Qt::Checked);

  // ui->customPlot->legend->setVisible(true);
  // give the axes some labels:
  ui->customPlot->xAxis->setLabel("motor speed (m/s)");
  ui->customPlot->yAxis->setLabel("gain");
  connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
  connect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMove(QMouseEvent*)));
  connect(ui->customPlot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease(QMouseEvent*)));
  connect(ui->customPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(mouseDoubleClick(QMouseEvent*)));
  ui->widgetCustom->setVisible(false);
  systemFunctionLabel = new QLabel("Current System Function: Windows Default");
  ui->statusBar->addWidget(systemFunctionLabel);
  ui->customPlot->replot();
}

MainWindow::~MainWindow()
{
  delete func;
  delete output;
  for (PointingDevice *inp : inputs)
    delete inp;
  delete ui;
}

void MainWindow::mousePress(QMouseEvent *event)
{
  double inputSpeed = this->ui->customPlot->xAxis->pixelToCoord(event->pos().x());
  double gain = this->ui->customPlot->yAxis->pixelToCoord(event->pos().y());
  double maxInputSpeed = 128. / input->getResolution() * ipm * input->getUpdateFrequency();

  if (customGraph)
  {
    for (auto it = customGraph->data()->begin(); it != customGraph->data()->end(); it++)
    {
      if (abs(inputSpeed - it->key) / maxInputSpeed < 0.02 && abs(gain - it->value) / maxGain < 0.02)
      {
        movedKey = it->key;
        return;
      }
    }
  }

  qDebug() << inputSpeed << gain;
}

void MainWindow::mouseMove(QMouseEvent *event)
{
  double inputSpeed = this->ui->customPlot->xAxis->pixelToCoord(event->pos().x());
  double gain = this->ui->customPlot->yAxis->pixelToCoord(event->pos().y());

  if (movedKey > 0)
  {
    customGraph->removeData(movedKey);
    customGraph->addData(inputSpeed, gain);
    movedKey = inputSpeed;
    replot();
  }
}

void MainWindow::mouseRelease(QMouseEvent *)
{
  movedKey = -1.;
}

void MainWindow::mouseDoubleClick(QMouseEvent *event)
{
  if (customGraph)
  {
    double inputSpeed = this->ui->customPlot->xAxis->pixelToCoord(event->pos().x());
    double gain = this->ui->customPlot->yAxis->pixelToCoord(event->pos().y());

    qDebug() << inputSpeed << gain;
    if (inputSpeed > 0 && gain > 0)
    {
      customGraph->addData(inputSpeed, gain);
      ui->customPlot->replot();
    }
  }
}

void MainWindow::changeShowPlot(TF &tf)
{
  QCustomPlot *customPlot = ui->customPlot;
  if (tf.shown)
  {
    QVector<double> x(128), y(128);
    for (int i = 0; i < 128; ++i)
    {
      x[i] = i / input->getResolution() * ipm * input->getUpdateFrequency();
      y[i] = tf.points[i];
    }
    customPlot->graph(tf.plotIndex)->setData(x, y);
    // There are 12 different colors starting from Qt::red position
    QColor color(Qt::GlobalColor(Qt::red + tf.plotIndex % 12));
    ui->customPlot->graph(tf.plotIndex)->setPen(QPen(color));
  }
  else
    customPlot->graph(tf.plotIndex)->clearData();
  customPlot->yAxis->setRange(0, maxGain);
}

void MainWindow::replot()
{
  for (TF & tf : allTfs)
    changeShowPlot(tf);
  ui->customPlot->replot();
}

TF *MainWindow::findFunc(string fName)
{
  for (TF &tf : allTfs)
  {
    if (tf.name == fName)
      return &tf;
  }
  return nullptr;
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
  TF *tf = findFunc(item->text().toLocal8Bit().constData());
  if (tf)
  {
    tf->shown = (item->checkState() == Qt::Checked);
    changeShowPlot(*tf);
    ui->customPlot->replot();
  }
  ui->lineEdit->setText(QString::fromStdString(tf->uri));
  ui->lineEdit->setCursorPosition(0);
  curName = tf->name;
}

void MainWindow::on_allButton_clicked()
{
  for(int i = 0; i < ui->listWidget->count(); i++)
  {
    QListWidgetItem * item = ui->listWidget->item(i);
    item->setCheckState(Qt::Checked);
  }
  for (TF & tf : allTfs)
    tf.shown = true;
  replot();
}

void MainWindow::on_noneButton_clicked()
{
  for(int i = 0; i < ui->listWidget->count(); i++)
  {
    QListWidgetItem * item = ui->listWidget->item(i);
    item->setCheckState(Qt::Unchecked);
  }
  for (TF & tf : allTfs)
    tf.shown = false;
  replot();
}

void MainWindow::on_changeButton_clicked()
{
  TF *tf = findFunc(curName);
  if (tf)
  {
    tf->uri = ui->lineEdit->text().toLocal8Bit().constData();
    recomputeFunc(*tf);
    changeShowPlot(*tf);
    updateInput();
    ui->customPlot->replot();
  }
}

LRESULT CALLBACK MainWindow::MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (nCode >= 0)
  {
    if (wParam == WM_MOUSEMOVE && func)
    {
      return -1;
    }
  }
  return CallNextHookEx(0, nCode, wParam, lParam);
}

void MainWindow::pointingCallback(void *, TimeStamp::inttime timestamp,
     int input_dx, int input_dy, int)
{
  int output_dx = 0, output_dy = 0;
  if (MainWindow::func)
  {
    MainWindow::func->applyi(input_dx, input_dy, &output_dx, &output_dy, timestamp);
    POINT p;
    GetCursorPos(&p);
    SetCursorPos(p.x + output_dx, p.y + output_dy);
  }
}

void MainWindow::on_applyButton_clicked()
{
  delete func;
  TF *tf = findFunc(curName);
  if (tf)
  {
    func = TransferFunction::create(tf->uri, input, output);
    systemFunctionLabel->setText("Current System Function: " + QString::fromStdString(func->getURI().asString()));
  }
}

void MainWindow::on_customButton_clicked()
{
  if (ui->widgetCustom->isVisible())
  {
    ui->customPlot->removeGraph(customGraph);
    ui->widgetCustom->setVisible(false);
    ui->customButton->setText("Custom");
    replot();
    customGraph = nullptr;
  }
  else
  {
    customGraph = ui->customPlot->addGraph();
    customGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
    ui->widgetCustom->setVisible(true);
    ui->customButton->setText("Cancel");
  }
}

void MainWindow::on_pushButtonAdd_clicked()
{
  // Convert points on the plot to counts and pixels
  map<int, double> resPoints;
  resPoints[0] = 0.;
  for (auto it = customGraph->data()->begin(); it != customGraph->data()->end(); it++)
  {
    int counts = it->key * input->getResolution() / ipm / input->getUpdateFrequency();
    if (counts > 0 && counts < 128)
    {
      double pixels = it->value * it->key * output->getResolution() / ipm / input->getUpdateFrequency();
      if (pixels > 0)
      {
        resPoints[counts] = pixels;
      }
    }
  }

  // Create files
  ConfigDict cd;
  cd.set("libpointing-input", input->getURI(true, true));
  cd.set("libpointing-output", output->getURI(true));
  cd.set("functions", "f");
  cd.set("default-function", "f");

  string name = ui->lineEditCustom->text().toStdString();

  string outputDir = moduleHeadersPath() + "\\pointing-echomouse\\" + name;

  if (CreateDirectoryA(outputDir.c_str(), NULL) ||
      ERROR_ALREADY_EXISTS == GetLastError())
  {
    ofstream file;
    string configDict = outputDir + "/config.dict";
    file.open(configDict.c_str(), ios::out | ios::trunc);

    file << cd.dump("\n", ":") << endl;
    file.close();

    string dat = outputDir + "/f.dat";
    file.open(dat.c_str(), ios::out | ios::trunc);

    int maxCount = resPoints.rbegin()->first;
    for (auto &kv : resPoints)
    {
      file << kv.first << ": " << kv.second << endl;
    }

    file << "max-counts: " <<  maxCount << endl;

    file.close();
  }
  else
  {
    cerr << "Failed to create the directory" << endl;
  }

  // Add new transfer function into the list
  ui->customPlot->removeGraph(customGraph);
  ui->widgetCustom->setVisible(false);
  ui->customButton->setText("Custom");
  customGraph = nullptr;
  updateList();
  replot();
}

void MainWindow::updateInput()
{
  ui->lineEditInput->setText(QString::fromStdString(input->getURI(true).asString()));
  double maxInputSpeed = 128. / input->getResolution() * ipm * input->getUpdateFrequency();
  ui->customPlot->xAxis->setRange(0, maxInputSpeed);
}

void MainWindow::on_pushButtonChangeInputURI_clicked()
{
  for (PointingDevice *inp : inputs)
  {
    if (inp->getURI().path == input->getURI().path)
    {
      inputs.remove(input);
      delete input;
      input = PointingDevice::create(ui->lineEditInput->text().toStdString());
      input->setPointingCallback(pointingCallback, nullptr);

      inputs.push_back(input);
      updateInput();
      recomputeAllFuncs();
      replot();
      break;
    }
  }
}

void MainWindow::on_comboBoxInput_currentIndexChanged(int index)
{
  QString uriString = ui->comboBoxInput->itemData(index).toString();
  URI uri(uriString.toStdString());
  for (PointingDevice *inp : inputs)
  {
    if (inp->getURI().path == uri.path)
    {
      input = inp;
      updateInput();
      break;
    }
  }
  recomputeAllFuncs();
  replot();
}
