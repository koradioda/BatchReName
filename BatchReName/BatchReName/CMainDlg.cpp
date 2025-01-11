
// CMainDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "BatchReName.h"
#include "CMainDlg.h"
#include "afxdialogex.h"
#include <thread>
#include <functional>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainDlg 对话框



CMainDlg::CMainDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BATCHRENAME_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

}

//  if(checkDirectory.GetCheck() == BST_CHECKED)  BST_UNCHECKED
void CMainDlg::batchReplaceInFileName(const std::string directoryPath, const std::string sourceStr, const std::string targetStr)
{
	string searchPath = directoryPath + "\\*.*";
	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  // 是文件夹，则递归调用，修改文件名
				{
					string newDirectoryPath = directoryPath + "\\" + findData.cFileName;
					batchReplaceInFileName(newDirectoryPath, sourceStr, targetStr);
				}
				else {  // 去掉 else 同时修改目录
					string fileNamePart = findData.cFileName;
					string oldFileName = directoryPath + "\\" + findData.cFileName;  // 旧名字
					size_t pos = 0;
					while ((pos = fileNamePart.find(sourceStr, pos)) != string::npos) {  // 仅仅替换文件名的字符串。
						fileNamePart.replace(pos, sourceStr.length(), targetStr);
						pos += targetStr.length();
					}
					string newFileName = directoryPath + "\\" + fileNamePart;  // 新名字

					if (MoveFileA(oldFileName.c_str(), newFileName.c_str()) == 0) {  // 重命名
						string msg = "重命名失败: " + oldFileName;
						CString messge(msg.c_str());
						AfxMessageBox(messge);
					}
				}
			}
		} while (FindNextFileA(hFind, &findData) != 0);

		FindClose(hFind);
	}
	else {
		string msg = "无法打开目录: " + directoryPath;
		CString messge(msg.c_str());
		AfxMessageBox(messge);
	}


}

BEGIN_MESSAGE_MAP(CMainDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_MODIFY, &CMainDlg::OnBnClickedModify)
END_MESSAGE_MAP()


// CMainDlg 消息处理程序

BOOL CMainDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	SetDlgItemText(IDC_PATH,_T(""));
	SetDlgItemText(IDC_SOURCE_STRING, _T(""));
	SetDlgItemText(IDC_TARGET_STRING, _T(""));

	// 设置 slogon 字体和大小
	CFont font;
	font.CreatePointFont(240, _T("Webdings"), NULL);
	auto pSlogon = (CStatic*)GetDlgItem(IDC_SLOGON);
	if (pSlogon != NULL)
	{
		pSlogon->SetFont(&font);
	}


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMainDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMainDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMainDlg::OnBnClickedModify()
{
	// TODO: 在此添加控件通知处理程序代码
	CString cPathString;
	// auto pPathEdit = (CEdit*)GetDlgItem(IDC_PATH);
	GetDlgItemText(IDC_PATH, cPathString);  // 获取路径
	if (cPathString == "" || cPathString == " ")
	{
		AfxMessageBox(_T("路径不能为空！"));
		return;
	}

	CString cSourceString;
	GetDlgItemText(IDC_SOURCE_STRING,cSourceString);  // 获取字符串源字符串
	if (cSourceString == "" || cSourceString == " ")
	{
		AfxMessageBox(_T("源字符串不能为空！"));
		return;
	}

	CString cTargetString;
	GetDlgItemText(IDC_TARGET_STRING, cTargetString);  // 获取目标字符串

	string directoryPath = _bstr_t(cPathString);
	string sourceStr = _bstr_t(cSourceString);
	string targetStr = _bstr_t(cTargetString);

	// 启动修改线程，防止界面卡顿
	thread bacthReName(std::bind(&CMainDlg::batchReplaceInFileName, this, directoryPath, sourceStr, targetStr));

	// 启动线程时，按钮无法点击
	auto pModifyButton = (CButton*)GetDlgItem(IDC_MODIFY);
	pModifyButton->EnableWindow(false);
	// 等待线程结束
	bacthReName.join();
	// 恢复按钮
	pModifyButton->EnableWindow(true);

	
}
