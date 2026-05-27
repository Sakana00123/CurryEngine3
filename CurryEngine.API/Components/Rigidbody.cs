
using CurryEngine.Interop;

namespace CurryEngine
{
    /// <summary>
    /// AddForce などの力の加え方を指定する列挙型。
    /// </summary>
    public enum ForceMode
    {
        Force = 0,
        Impulse = 1,
        VelocityChange = 2,
        Acceleration = 3
    }


    public sealed class Rigidbody : Component
    {
        public float mass
        {
            get => NativeMethods.Rigidbody_GetMass(ownerId);
            set => NativeMethods.Rigidbody_SetMass(ownerId, value);
        }
        public Vector3 velocity
        {
            get => NativeMethods.Rigidbody_GetVelocity(ownerId);
            set => NativeMethods.Rigidbody_SetVelocity(ownerId, value);
        }

        public Vector3 angularVelocity
        {
            get => NativeMethods.Rigidbody_GetAngularVelocity(ownerId);
            set => NativeMethods.Rigidbody_SetAngularVelocity(ownerId, value);
        }

        public bool useGravity
        {
            get => NativeMethods.Rigidbody_GetUseGravity(ownerId);
            set => NativeMethods.Rigidbody_SetUseGravity(ownerId, value);
        }

        public bool isKinematic
        {
            get => NativeMethods.Rigidbody_GetIsKinematic(ownerId);
            set => NativeMethods.Rigidbody_SetIsKinematic(ownerId, value);
        }

        public void AddForce(Vector3 force, ForceMode mode = ForceMode.Force)
        {
            NativeMethods.Rigidbody_AddForce(ownerId, force, (int)mode);
        }
        public void AddTorque(Vector3 torque, ForceMode mode = ForceMode.Force)
        {
            NativeMethods.Rigidbody_AddTorque(ownerId, torque, (int)mode);
        }

        public void SetKinematicTarget(Vector3 position, Quaternion rotation)
        {
            NativeMethods.Rigidbody_SetKinematicTarget(ownerId, position, rotation);
        }



    }
}
