# curve_plot
## 说明
鉴于没找到好看且好用的画roc的工具，特用[某人](https://github.com/zjysnow/ChaosLab)写的工具来画roc，简单好用
具体例子
```cpp
auto figure = PlotFigure::Figure(); // 申请实例
figure->Plot(x1, y1, "LineWidth", 2, "Color", ColorPool::Get(LIME), "Legend", "STEM"); // 画 第一个
figure->Hold(ON); //可以画第二幅图 在同一画布下
figure->Plot(x2, y2, "LineWidth", 2, "Color", ColorPool::Get(RED), "Legend", "DREAM");  // 画第二个图
figure->Set("YLabel", "Tpr", "XLabel", "Fpr", "Title", "ROC", "XRange", cv::Vec2f(0, max_xrange));
figure->Show();    
```
###ps： 当然除了roc之类的很多线型图也可以画，具体可以看代码，只要是一对一的x，y都可以