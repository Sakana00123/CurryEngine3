using CurryEngine.Interop;

namespace CurryEngine
{
    public sealed class Collider : Component
    {


        /// <summary>
        /// このコライダーがトリガーかどうか。
        /// </summary>
        public bool isTrigger
        {
            get => NativeMethods.Collider_GetIsTrigger(ownerId, objectId);
            set => NativeMethods.Collider_SetIsTrigger(ownerId, objectId, value);
        }
    }
}
