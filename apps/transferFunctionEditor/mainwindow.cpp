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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <pointing/utils/FileUtils.h>
#include <pointing/utils/ConfigDict.h>
#include <fstream>

using namespace std;
using namespace pointing;

#define ipm 0.0254f

pointing::TransferFunction *MainWindow::func = NULL;

void MainWindow::recomputeFunc(TF &tf)
{
  tf.points = vector<double>(128, 0.);
  TransferFunction *f = TransferFunction::create(tf.uri, input, output);
  for (unsigned i = 1; i < tf.points.size(); i++)
  {
    f->clearState();
    double x, y;
    // FIXME Timestamp + delta
    f->applyd(i, 0, &x, &y);
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
  for (TF &tf : allTfs)
    recomputeFunc(tf);
}

list<TF> MainWindow::getDefaultTfs()
{
  list<TF> result;
  string dir = moduleHeadersPath() + "\\pointing-echomouse\\";
  result.push_back(TF("Windows", "windows:?", TFTypes::Echomouse, "windows\\epp"));
  result.push_back(TF("Sigmoid", "sigmoid:?", TFTypes::Default));
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

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  input = PointingDevice::create("winhid:/65595?debugLevel=1&cpi=400&hz=1000");
  output = DisplayDevice::create("any:?");
  curName = "Windows";

  SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);
  input->setPointingCallback(pointingCallback, nullptr);

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
  // set axes ranges, so we see all data:
  double maxInputSpeed = 128. / input->getResolution() * ipm * input->getUpdateFrequency();
  ui->customPlot->xAxis->setRange(0, maxInputSpeed);
  connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress(QMouseEvent*)));
  connect(ui->customPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(mouseDoubleClick(QMouseEvent*)));
  ui->widgetCustom->setVisible(false);
}

MainWindow::~MainWindow()
{
  delete func;
  delete output;
  delete input;
  delete ui;
}

void MainWindow::mousePress(QMouseEvent *event)
{
  double inputSpeed = this->ui->customPlot->xAxis->pixelToCoord(event->pos().x());
  double gain = this->ui->customPlot->yAxis->pixelToCoord(event->pos().y());

  qDebug() << inputSpeed << gain;
}

void MainWindow::mouseDoubleClick(QMouseEvent *event)
{
  if (customGraph)
  {
    double inputSpeed = this->ui->customPlot->xAxis->pixelToCoord(event->pos().x());
    double gain = this->ui->customPlot->yAxis->pixelToCoord(event->pos().y());

    qDebug() << inputSpeed << gain;
    customGraph->addData(inputSpeed, gain);
    ui->customPlot->replot();
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
  customPlot->replot();
}

void MainWindow::replot()
{
  for (TF & tf : allTfs)
    changeShowPlot(tf);
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
    ui->systemFunctionLabel->setText(QString::fromStdString(func->getURI().asString()));
  }
}

void MainWindow::saveTransferFunction()
{
  /*
  ConfigDict cd;
  cd.set("libpointing-input", input->getURI(true, true));
  cd.set("libpointing-output", output->getURI(true));
  cd.set("functions", "f");
  cd.set("default-function", "f");

  string outputDir = moduleHeadersPath() + "/pointing-echomouse/" + ui->lineEdit->text().toStdString();

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

    double coef = output->getResolution() / 0.0254f / input->getUpdateFrequency();

    int maxCount = 0;
    for (unsigned i = 0; i < ui->graphicsView->customPoints.size(); i++)
    {
      double point = ui->graphicsView->customPoints[i];
      if (point >= 0)
      {
        file << i << ": " << point * coef << endl;
        maxCount = i;
      }
    }

    file << "max-counts: " <<  maxCount << endl;

    file.close();
  }
  else
  {
    cerr << "Failed to create the directory" << endl;
  }
  */
}

void MainWindow::on_customButton_clicked()
{
  if (ui->widgetCustom->isVisible())
  {
    ui->customPlot->removeGraph(customGraph);
    ui->widgetCustom->setVisible(false);
    ui->customButton->setText("Custom");
    customGraph = nullptr;
  }
  else
  {
    customGraph = ui->customPlot->addGraph();
    ui->widgetCustom->setVisible(true);
    ui->customButton->setText("Cancel");
  }
}
