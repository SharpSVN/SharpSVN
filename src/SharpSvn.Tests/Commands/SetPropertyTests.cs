// $Id$
// Copyright (c) SharpSvn Project 2008, Copyright (c) Ankhsvn 2003-2007
using System;
using System.Collections;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using NUnit.Framework;
using NUnit.Framework.SyntaxHelpers;
using SharpSvn;

namespace SharpSvn.Tests.Commands
{
    /// <summary>
    /// Tests Client::ClientPropSet
    /// </summary>
    [TestFixture]
    public class SetPropertyTests : TestBase
    {
        [SetUp]
        public override void SetUp()
        {
            base.SetUp();
            this.ExtractWorkingCopy();
        }

        /// <summary>
        ///Attempts to Set Properties on a file
        /// </summary>
        [Test]
        public void TestSetProp()
        {
            string filePath = Path.Combine(this.WcPath, "Form.cs");

            byte[] propval = Encoding.UTF8.GetBytes("baa");
            this.Client.SetProperty(filePath, "moo", propval);
            Assert.That(this.RunCommand("svn", "propget moo " + filePath).Trim(), Is.EqualTo("baa"),
                "PropSet didn't work!");
        }

        /// <summary>
        ///Attempts to set Properties on a directory recursively. 
        /// </summary>
        [Test]
        public void TestSetPropRecursivly()
        {
            string filePath = Path.Combine(this.WcPath, "Form.cs");

            byte[] propval = Encoding.UTF8.GetBytes("baa");
            bool ticked = false;
            SvnSetPropertyArgs a = new SvnSetPropertyArgs();
            a.Notify += delegate(object sender, SvnNotifyEventArgs e)
            {
                ticked = true;
            };
            a.Depth = SvnDepth.Infinity;

            this.Client.SetProperty(WcPath, "moo", propval, a);

            if (SvnClient.Version >= new Version(1, 6))
                Assert.That(ticked, Is.True);

            Assert.That(this.RunCommand("svn", "propget moo " + this.WcPath).Trim(), Is.EqualTo("baa"),
                "PropSet didn't work on directory!");

            Assert.That(this.RunCommand("svn", "propget moo " + filePath).Trim(), Is.EqualTo("baa"),
                "PropSet didn't work on file!");


        }

        void a_Notify(object sender, SvnNotifyEventArgs e)
        {
            throw new NotImplementedException();
        }

        //Test case to show up a bug in binary property retrieval.  
        [Test]
        public void TestPropSetGet()
        {
            Byte[] valueToSet = { 0, 77, 73, 65, 79, 87, 0 };
            //In ASCII looks like \0MIAOW\0 where \0 is a null/zero value 
            // as used for terminating c style strings

            string path = Path.Combine(this.WcPath, "Form.cs");
            this.Client.SetProperty(path, "TestBinaryCat", valueToSet);
            SvnPropertyValue ret;
            Assert.That(Client.GetProperty(new SvnPathTarget(path),
                "TestBinaryCat", out ret));

            Assert.That(ret.Key, Is.EqualTo("TestBinaryCat"));
            Assert.That(ret.Target.TargetName, Is.EqualTo(path));
            Assert.That(ret.RawValue,
                Is.EqualTo(valueToSet), "Binary property retrieved is different");
            Assert.That(ret.RawValue, Is.EqualTo(valueToSet),
                "Binary property as retrieved is different to that set");
        }

    }

}
