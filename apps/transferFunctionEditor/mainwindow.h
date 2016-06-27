/* -*- mode: c++ -*-
 *
 * apps/transferFunctionEditor/mainwindow.h --
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <pointing/pointing.h>
#include <windows.h>
#include <qcustomplot.h>

namespace Ui {
  class MainWindow;
}

enum class TFTypes {None, Default, Echomouse, Custom};

struct TF
{
  std::string name;
  std::string uri;
  TFTypes type = TFTypes::None;
  std::string path;
  std::vector<double> points;
  bool shown = false;
  int plotIndex = 0;

  TF(std::string name, std::string uri, TFTypes type, std::string path="")
    :name(name),uri(uri),type(type),path(path){}
};

class MainWindow : public QMainWindow
{
  Q_OBJECT

  void updateList();

  std::list<TF> getDefaultTfs();

  bool isInLibpointing(std::string path);

  std::list<TF> allTfs;

  void recomputeFunc(TF &tf);

  void recomputeAllFuncs();
  TF *findFunc(std::string fName);

  pointing::PointingDevice *input = NULL;
  pointing::DisplayDevice *output = NULL;
  static pointing::TransferFunction *func;
  std::list<pointing::PointingDevice *> inputs;

  std::string curName;

  double maxGain = 0;

  void changeShowPlot(TF &tf);
  void replot();

  static void pointingCallback(void *, pointing::TimeStamp::inttime timestamp,
       int input_dx, int input_dy, int);

  static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam);

  void saveTransferFunction();

  QCPGraph *customGraph = nullptr;
  QLabel *systemFunctionLabel;

  void setupInput();
  void updateInput();

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void on_listWidget_itemClicked(QListWidgetItem *item);

  void on_allButton_clicked();

  void mousePress(QMouseEvent *event);
  void mouseDoubleClick(QMouseEvent *event);

  void on_noneButton_clicked();

  void on_changeButton_clicked();

  void on_applyButton_clicked();

  void on_customButton_clicked();

  void on_pushButtonAdd_clicked();

  void on_pushButtonChangeInputURI_clicked();

  void on_comboBoxInput_currentIndexChanged(int index);

private:
  Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
