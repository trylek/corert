﻿// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System;
using System.Diagnostics;

using ILCompiler.DependencyAnalysis;

using Internal.JitInterface;
using Internal.Text;
using Internal.TypeSystem;

namespace ILCompiler.DependencyAnalysis.ReadyToRun
{
    public class MethodWithGCInfo : ObjectNode, IReadyToRunMethodCodeNode, IMethodBodyNode
    {
        public readonly MethodGCInfoNode GCInfoNode;

        private readonly MethodDesc _method;
        private readonly SignatureContext _signatureContext;

        private ObjectData _methodCode;
        private FrameInfo[] _frameInfos;
        private byte[] _gcInfo;
        private ObjectData _ehInfo;
        private OffsetMapping[] _debugLocInfos;
        private NativeVarInfo[] _debugVarInfos;
        private DebugEHClauseInfo[] _debugEHClauseInfos;

        public MethodWithGCInfo(MethodDesc methodDesc, SignatureContext signatureContext)
        {
            GCInfoNode = new MethodGCInfoNode(this);
            _method = methodDesc;
            _signatureContext = signatureContext;
        }

        public void SetCode(ObjectData data)
        {
            Debug.Assert(_methodCode == null);
            _methodCode = data;
        }

        public MethodDesc Method => _method;

        public override ObjectData GetData(NodeFactory factory, bool relocsOnly)
        {
            return _methodCode;
        }

        protected override DependencyList ComputeNonRelocationBasedDependencies(NodeFactory factory)
        {
            return new DependencyList(new DependencyListEntry[] { new DependencyListEntry(GCInfoNode, "Unwind & GC info") });
        }

        public override bool StaticDependenciesAreComputed => _methodCode != null;

        public virtual void AppendMangledName(NameMangler nameMangler, Utf8StringBuilder sb)
        {
            sb.Append(nameMangler.GetMangledMethodName(_method));
        }

        protected override string GetName(NodeFactory factory)
        {
            Utf8StringBuilder sb = new Utf8StringBuilder();
            AppendMangledName(factory.NameMangler, sb);
            return sb.ToString();
        }

        public override int ClassCode => 315213488;

        public override ObjectNodeSection Section
        {
            get
            {
                return _method.Context.Target.IsWindows ? ObjectNodeSection.ManagedCodeWindowsContentSection : ObjectNodeSection.ManagedCodeUnixContentSection;
            }
        }

        public FrameInfo[] FrameInfos => _frameInfos;
        public byte[] GCInfo => _gcInfo;
        public ObjectData EHInfo => _ehInfo;

        public ISymbolNode GetAssociatedDataNode(NodeFactory factory)
        {
            if (MethodAssociatedDataNode.MethodHasAssociatedData(factory, this))
                return factory.MethodAssociatedData(this);

            return null;
        }

        public void InitializeFrameInfos(FrameInfo[] frameInfos)
        {
            Debug.Assert(_frameInfos == null);
            _frameInfos = frameInfos;
        }

        public void InitializeGCInfo(byte[] gcInfo)
        {
            Debug.Assert(_gcInfo == null);
            _gcInfo = gcInfo;
        }

        public void InitializeEHInfo(ObjectData ehInfo)
        {
            Debug.Assert(_ehInfo == null);
            _ehInfo = ehInfo;
        }

        public OffsetMapping[] DebugLocInfos => _debugLocInfos;
        public NativeVarInfo[] DebugVarInfos => _debugVarInfos;
        public DebugEHClauseInfo[] DebugEHClauseInfos => _debugEHClauseInfos;

        public void InitializeDebugLocInfos(OffsetMapping[] debugLocInfos)
        {
            Debug.Assert(_debugLocInfos == null);
            _debugLocInfos = debugLocInfos;
        }

        public void InitializeDebugVarInfos(NativeVarInfo[] debugVarInfos)
        {
            Debug.Assert(_debugVarInfos == null);
            _debugVarInfos = debugVarInfos;
        }

        public void InitializeDebugEHClauseInfos(DebugEHClauseInfo[] debugEHClauseInfos)
        {
            Debug.Assert(_debugEHClauseInfos == null);
            _debugEHClauseInfos = debugEHClauseInfos;
        }

        public int CompareToImpl(ISortableSymbolNode other, CompilerComparer comparer)
        {
            throw new NotImplementedException();
        }

        public int Offset => 0;
        public override bool IsShareable => _method is InstantiatedMethod || EETypeNode.IsTypeNodeShareable(_method.OwningType);
    }
}