using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Injector
{
    public partial class MainForm : Form
    {

        private List<int> pids = new List<int>();
        
        public MainForm()
        {
            InitializeComponent();
        }

        private void btn_refresh_Click(object sender, EventArgs e)
        {
            pids.Clear();
            comboBox.Items.Clear();
            foreach (Process process in Process.GetProcessesByName("javaw"))
            {
                pids.Add(process.Id);
                comboBox.Items.Add($"{process.Id} - {process.MainWindowTitle}");
            }
            foreach (Process process in Process.GetProcessesByName("java"))
            {
                pids.Add(process.Id);
                comboBox.Items.Add($"{process.Id} - {process.MainWindowTitle}");
            }
        }

        private void btn_choose_Click(object sender, EventArgs e)
        {
            if(openFileDialog.ShowDialog() == DialogResult.OK)
            {
                textBox.Text = openFileDialog.FileName;
            }
        }

        private void btn_inject_Click(object sender, EventArgs e)
        {
            if (comboBox.SelectedIndex < 0)
                return;
            int pid = pids[comboBox.SelectedIndex];
            Injector.InjectDLL(pid, textBox.Text);
        }

        private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            Process.Start("rundll32.exe", "url.dll,FileProtocolHandler https://github.com/sduoduo233/LunarInputFix/");
        }
    }
}
