#include "MainWindow.h"

MainWindow::MainWindow(const juce::String& name)
    : DocumentWindow(name,
                      juce::Desktop::getInstance().getDefaultLookAndFeel()
                          .findColour(juce::ResizableWindow::backgroundColourId),
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);

    auto* content = new MainComponent();
    mainComponent = content;
    setContentOwned(content, true);

    menuBarModel.onOpenFileSelected = [this] { mainComponent->openFile(); };
    setMenuBar(&menuBarModel);

    centreWithSize(getWidth(), getHeight());
    setResizable(true, true);
    setVisible(true);

    mainComponent->grabKeyboardFocus();
}

MainWindow::~MainWindow()
{
    setMenuBar(nullptr);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}