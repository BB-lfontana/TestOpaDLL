namespace TestOpaDLL
{
    partial class FormMain
    {
        /// <summary>
        /// Variabile di progettazione necessaria.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Pulire le risorse in uso.
        /// </summary>
        /// <param name="disposing">ha valore true se le risorse gestite devono essere eliminate, false in caso contrario.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Codice generato da Progettazione Windows Form

        /// <summary>
        /// Metodo necessario per il supporto della finestra di progettazione. Non modificare
        /// il contenuto del metodo con l'editor di codice.
        /// </summary>
        private void InitializeComponent()
        {
            this.BtnOpenPort = new System.Windows.Forms.Button();
            this.BtnClosePort = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.lblValues = new System.Windows.Forms.Label();
            this.btnGASOpenPort = new System.Windows.Forms.Button();
            this.btnGASClosePort = new System.Windows.Forms.Button();
            this.label9 = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.lblAUX = new System.Windows.Forms.Label();
            this.ldbDateTime = new System.Windows.Forms.Label();
            this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
            this.btnSwitch = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // BtnOpenPort
            // 
            this.BtnOpenPort.Location = new System.Drawing.Point(56, 11);
            this.BtnOpenPort.Name = "BtnOpenPort";
            this.BtnOpenPort.Size = new System.Drawing.Size(119, 23);
            this.BtnOpenPort.TabIndex = 0;
            this.BtnOpenPort.Text = "Open OPA serial port";
            this.BtnOpenPort.UseVisualStyleBackColor = true;
            this.BtnOpenPort.Click += new System.EventHandler(this.BtnOpenPort_Click);
            // 
            // BtnClosePort
            // 
            this.BtnClosePort.Location = new System.Drawing.Point(205, 11);
            this.BtnClosePort.Name = "BtnClosePort";
            this.BtnClosePort.Size = new System.Drawing.Size(164, 23);
            this.BtnClosePort.TabIndex = 0;
            this.BtnClosePort.Text = "Close OPA serial port";
            this.BtnClosePort.UseVisualStyleBackColor = true;
            this.BtnClosePort.Click += new System.EventHandler(this.BtnClosePort_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("JetBrains Mono", 12F, System.Drawing.FontStyle.Bold);
            this.label1.Location = new System.Drawing.Point(56, 77);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(70, 21);
            this.label1.TabIndex = 1;
            this.label1.Text = "label1";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("JetBrains Mono", 12F, System.Drawing.FontStyle.Bold);
            this.label2.Location = new System.Drawing.Point(56, 101);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(70, 21);
            this.label2.TabIndex = 2;
            this.label2.Text = "label2";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Font = new System.Drawing.Font("JetBrains Mono", 12F, System.Drawing.FontStyle.Bold);
            this.label3.Location = new System.Drawing.Point(56, 124);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(70, 21);
            this.label3.TabIndex = 2;
            this.label3.Text = "label2";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Font = new System.Drawing.Font("JetBrains Mono", 12F, System.Drawing.FontStyle.Bold);
            this.label4.Location = new System.Drawing.Point(56, 146);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(70, 21);
            this.label4.TabIndex = 2;
            this.label4.Text = "label2";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Font = new System.Drawing.Font("JetBrains Mono", 12F, System.Drawing.FontStyle.Bold);
            this.label5.Location = new System.Drawing.Point(56, 169);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(70, 21);
            this.label5.TabIndex = 2;
            this.label5.Text = "label2";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Font = new System.Drawing.Font("JetBrains Mono", 12F, System.Drawing.FontStyle.Bold);
            this.label6.Location = new System.Drawing.Point(56, 193);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(70, 21);
            this.label6.TabIndex = 2;
            this.label6.Text = "label2";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Font = new System.Drawing.Font("JetBrains Mono SemiBold", 12F, System.Drawing.FontStyle.Bold);
            this.label7.Location = new System.Drawing.Point(56, 352);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(70, 21);
            this.label7.TabIndex = 3;
            this.label7.Text = "label7";
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Font = new System.Drawing.Font("JetBrains Mono SemiBold", 12F, System.Drawing.FontStyle.Bold);
            this.label8.Location = new System.Drawing.Point(56, 376);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(70, 21);
            this.label8.TabIndex = 4;
            this.label8.Text = "label8";
            // 
            // lblValues
            // 
            this.lblValues.AutoSize = true;
            this.lblValues.Font = new System.Drawing.Font("Source Code Pro", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblValues.Location = new System.Drawing.Point(153, 77);
            this.lblValues.Name = "lblValues";
            this.lblValues.Size = new System.Drawing.Size(116, 81);
            this.lblValues.TabIndex = 5;
            this.lblValues.Text = "label923\r\n4\r\n234234";
            this.lblValues.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // btnGASOpenPort
            // 
            this.btnGASOpenPort.Location = new System.Drawing.Point(56, 40);
            this.btnGASOpenPort.Name = "btnGASOpenPort";
            this.btnGASOpenPort.Size = new System.Drawing.Size(119, 23);
            this.btnGASOpenPort.TabIndex = 6;
            this.btnGASOpenPort.Text = "Open GAS serial port";
            this.btnGASOpenPort.UseVisualStyleBackColor = true;
            this.btnGASOpenPort.Click += new System.EventHandler(this.btnGasComPort_Click);
            // 
            // btnGASClosePort
            // 
            this.btnGASClosePort.Location = new System.Drawing.Point(205, 40);
            this.btnGASClosePort.Name = "btnGASClosePort";
            this.btnGASClosePort.Size = new System.Drawing.Size(164, 23);
            this.btnGASClosePort.TabIndex = 7;
            this.btnGASClosePort.Text = "Close GAS serial port";
            this.btnGASClosePort.UseVisualStyleBackColor = true;
            this.btnGASClosePort.Click += new System.EventHandler(this.btnGASClosePort_Click);
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Font = new System.Drawing.Font("JetBrains Mono SemiBold", 12F, System.Drawing.FontStyle.Bold);
            this.label9.Location = new System.Drawing.Point(56, 402);
            this.label9.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(70, 21);
            this.label9.TabIndex = 8;
            this.label9.Text = "label9";
            // 
            // label10
            // 
            this.label10.AutoSize = true;
            this.label10.Font = new System.Drawing.Font("JetBrains Mono SemiBold", 12F, System.Drawing.FontStyle.Bold);
            this.label10.Location = new System.Drawing.Point(56, 430);
            this.label10.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(80, 21);
            this.label10.TabIndex = 9;
            this.label10.Text = "label10";
            // 
            // lblAUX
            // 
            this.lblAUX.AutoSize = true;
            this.lblAUX.Font = new System.Drawing.Font("Source Code Pro", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lblAUX.Location = new System.Drawing.Point(139, 248);
            this.lblAUX.Name = "lblAUX";
            this.lblAUX.Size = new System.Drawing.Size(116, 81);
            this.lblAUX.TabIndex = 10;
            this.lblAUX.Text = "label923\r\n4\r\n234234";
            this.lblAUX.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // ldbDateTime
            // 
            this.ldbDateTime.AutoSize = true;
            this.ldbDateTime.Font = new System.Drawing.Font("Source Code Pro", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.ldbDateTime.Location = new System.Drawing.Point(319, 462);
            this.ldbDateTime.Name = "ldbDateTime";
            this.ldbDateTime.Size = new System.Drawing.Size(155, 27);
            this.ldbDateTime.TabIndex = 11;
            this.ldbDateTime.Text = "GASDateTime";
            this.ldbDateTime.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // btnSwitch
            // 
            this.btnSwitch.Location = new System.Drawing.Point(110, 467);
            this.btnSwitch.Name = "btnSwitch";
            this.btnSwitch.Size = new System.Drawing.Size(130, 23);
            this.btnSwitch.TabIndex = 12;
            this.btnSwitch.Text = "Switch BT100";
            this.btnSwitch.UseVisualStyleBackColor = true;
            this.btnSwitch.Click += new System.EventHandler(this.button1_Click);
            // 
            // FormMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1188, 520);
            this.Controls.Add(this.btnSwitch);
            this.Controls.Add(this.ldbDateTime);
            this.Controls.Add(this.lblAUX);
            this.Controls.Add(this.label10);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.btnGASClosePort);
            this.Controls.Add(this.btnGASOpenPort);
            this.Controls.Add(this.lblValues);
            this.Controls.Add(this.label8);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.BtnClosePort);
            this.Controls.Add(this.BtnOpenPort);
            this.Name = "FormMain";
            this.Text = "DLL Opa Drver";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button BtnOpenPort;
        private System.Windows.Forms.Button BtnClosePort;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label8;
        private System.Windows.Forms.Label lblValues;
        private System.Windows.Forms.Button btnGASOpenPort;
        private System.Windows.Forms.Button btnGASClosePort;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Label label10;
        private System.Windows.Forms.Label lblAUX;
        private System.Windows.Forms.Label ldbDateTime;
        private System.Windows.Forms.SaveFileDialog saveFileDialog1;
        private System.Windows.Forms.Button btnSwitch;
    }
}

